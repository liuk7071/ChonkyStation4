#include "ShaderDecompiler.hpp"
#include <GCN/Shader/Decoder.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/VSharp.hpp>
#include <sstream>
#include <unordered_map>


namespace PS4::GCN::Shader {

std::string shader;
std::unordered_map<int, std::string> in_attrs;
std::unordered_map<int, std::string> out_attrs;

void addFunc(std::string name, std::string body) {
    shader += name + "() {\n";
    std::istringstream stream(body);
    for (std::string line; std::getline(stream, line); )
        shader += "\t" + line + "\n";
    shader += "}\n";
}

void addInAttr(std::string name, std::string type, int location) {
    if (in_attrs.contains(location)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(in_attrs[location] == name, "ShaderDecompiler: tried to add an input attribute to an already used location with a different name\n");
        return;
    }
    in_attrs[location] = name;
    shader += std::format("layout(location = {}) in {} {};\n", location, type, name);
}

void addOutAttr(std::string name, std::string type, int location) {
    if (out_attrs.contains(location)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(out_attrs[location] == name, "ShaderDecompiler: tried to add an output attribute to an already used location with a different name\n");
        return;
    }
    out_attrs[location] = name;
    shader += std::format("layout(location = {}) out {} {};\n", location, type, name);
}

std::string getType(int n_lanes) {
    switch (n_lanes) {
    case 1: return "float";
    case 2: return "vec2";
    case 3: return "vec3";
    case 4: return "vec4";
    default: Helpers::panic("ShaderDecompiler: getType: invalid n_lanes (%d)\n", n_lanes);
    }
}

std::string decompileShader(u32* data, ShaderStage stage, FetchShader* fetch_shader) {
    Shader::GcnDecodeContext decoder;
    Shader::GcnCodeSlice code_slice((u32*)data, data + std::numeric_limits<u32>::max());

    shader.clear();
    shader.reserve(16_KB);  // Avoid reallocations
    in_attrs.clear();
    out_attrs.clear();

    shader += R"(
#version 410 core
#extension GL_ARB_shading_language_packing : require

float s[104];
float v[104];

)";

    std::string main;
    main.reserve(1_KB); // Avoid reallocations

    switch (stage) {
    case ShaderStage::Vertex: {
        // Write vertex inputs from fetch shader
        for (auto& binding : fetch_shader->bindings) {
            VSharp* vsharp = binding.vsharp_loc.asPtr();
            std::string attr = std::format("vs_attr{}", binding.dest_vgpr);
            addInAttr(attr, getType(binding.n_elements), binding.dest_vgpr);

            // In main(), move the input data to the VGPRs.
            // Handle swizzling
            std::array<u32, 4> swizzles = { vsharp->dst_sel_x, vsharp->dst_sel_y, vsharp->dst_sel_z, vsharp->dst_sel_w };
            for (int i = 0; i < binding.n_elements; i++) {
                auto swizzle = swizzles[i];
                std::string val;
                if      (swizzle == DSEL_0) val = "0.0f";
                else if (swizzle == DSEL_1) val = "1.0f";
                else if (swizzle == DSEL_R) val = attr + ".x";
                else if (swizzle == DSEL_G) val = attr + ".y";
                else if (swizzle == DSEL_B) val = attr + ".z";
                else if (swizzle == DSEL_A) val = attr + ".w";

                main += std::format("v[{}] = {};\n", binding.dest_vgpr + i, val);
            }
        }
        break;
    }
    }

    main += "\n";

    bool done = false;
    while (!code_slice.atEnd() && !done) {
        const auto instr = decoder.decodeInstruction(code_slice);

        switch (instr.opcode) {
        case Shader::Opcode::S_ENDPGM: {
            done = true;
            continue;
        }

        case Shader::Opcode::S_MOV_B32: {
            if (instr.dst[0].code > 104) {
                main += "// TODO: S_MOV_B32 to special register\n";
                continue;
            }
            Helpers::panic("S_MOV_B32 to a normal register (TODO)\n");
            break;
        }

        case Shader::Opcode::S_SWAPPC_B64: {
            main += "// S_SWAPPC_B64\n";
            break;
        }

        case Shader::Opcode::V_CVT_PKRTZ_F16_F32: {
            main += std::format("v[{}] = uintBitsToFloat(packHalf2x16(vec2(v[{}], v[{}])));\n", instr.dst[0].code, instr.src[0].code, instr.src[1].code);
            break;
        }

        case Shader::Opcode::V_MOV_B32: {
            std::string src;
            if (instr.src[0].code <= 104) src = std::format("s[{}]", instr.src[0].code);
            else if (instr.src[0].code == 128) src = "0.0f";
            else if (instr.src[0].code == 242) src = "1.0f";
            else Helpers::panic("V_MOV_B32 from unhandled source register %d\n", instr.src[0].code);

            main += std::format("v[{}] = {};\n", instr.dst[0].code, src);
            break;
        }

        case Shader::Opcode::V_INTERP_P1_F32: {
            const std::string attr = std::format("ps_attr{}", instr.control.vintrp.attr);
            addInAttr(attr, "vec4", instr.control.vintrp.attr);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            main += std::format("v[{}] = {}.{}; // V_INTERP_P1_F32\n", instr.dst[0].code, attr, lanes[instr.control.vintrp.chan]);
            break;
        }

        case Shader::Opcode::V_INTERP_P2_F32: {
            const std::string attr = std::format("ps_attr{}", instr.control.vintrp.attr);
            addInAttr(attr, "vec4", instr.control.vintrp.attr);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            main += std::format("v[{}] = {}.{}; // V_INTERP_P2_F32\n", instr.dst[0].code, attr, lanes[instr.control.vintrp.chan]);
            break;
        }

        case Shader::Opcode::EXP: {
            const auto tgt = instr.control.exp.target;
            
            // When compr is enabled, EXP uses four 16bit values packed in VSRC0 and VSRC1 instead of four 32bit values in VSRC0, VSRC1, VSRC2 and VSRC3
            std::string data;
            if (!instr.control.exp.compr)
                data = std::format("vec4(v[{}], v[{}], v[{}], v[{}])", instr.src[0].code, instr.src[1].code, instr.src[2].code, instr.src[3].code);
            else
                data = std::format("vec4(unpackHalf2x16(floatBitsToUint(v[{}])), unpackHalf2x16(floatBitsToUint(v[{}])))", instr.src[0].code, instr.src[1].code);

            // Color targets
            if (tgt >= 0 && tgt <= 8) {
                const std::string attr = std::format("col{}", tgt);
                addOutAttr(attr, "vec4", tgt);
                main += std::format("{} = {};\n", attr, data);
            }
            // Output pos0
            else if (tgt == 12) {
                main += "gl_Position = " + data + ";\n";
            }
            // Output attribute
            else if (tgt >= 32 && tgt < 64) {
                const std::string attr = std::format("ps_attr{}", tgt - 32);
                addOutAttr(attr, "vec4", tgt - 32);
                main += std::format("{} = {};\n", attr, data);
            }
            else Helpers::panic("ShaderDecompiler: EXP unhandled tgt %d\n", tgt);
            break;
        }

        default: {
            Helpers::panic("Unimplemented shader instruction %d\n", instr.opcode);
        }
        }
    }

    shader += "\n";

    // Declare main function
    addFunc("void main", main);

    printf("%s\n", shader.c_str());
    return shader;
}

}   // End namespace PS4::GCN::Shader
