#include "ShaderDecompiler.hpp"
#include <GCN/Shader/Decoder.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/GCN.hpp>
#include <sstream>
#include <unordered_map>
#include <deque>


namespace PS4::GCN::Shader {

std::string shader;
std::string const_tables;
std::unordered_map<std::string, size_t> const_table_map;
std::unordered_map<int, std::string> in_attrs;
std::unordered_map<int, std::string> out_attrs;
std::unordered_map<int, std::string> in_buffers;

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

void addInSSBO(std::string name, int binding) {
    if (in_buffers.contains(binding)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(in_buffers[binding] == name, "ShaderDecompiler: tried to add an input SSBO to an already used binding with a different name\n");
        return;
    }
    in_buffers[binding] = name;
    shader += std::format("layout(binding = {}, std430) readonly buffer {}_t {{ uint data[]; }} {};\n", binding, name, name);
}

void addInSampler2D(std::string name, int binding) {
    if (in_buffers.contains(binding)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(in_buffers[binding] == name, "ShaderDecompiler: tried to add an input sampelr2D to an already used binding with a different name\n");
        return;
    }
    in_buffers[binding] = name;
    shader += std::format("layout(binding = {}) uniform sampler2D {};\n", binding, name);
}

void addFloatConstTable(std::string name, float* table, size_t size) {
    if (const_table_map.contains(name)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(const_table_map[name] == size, "ShaderDecompiler: tried to add a const table with same name but different size\n");
        return;
    }
    const_table_map[name] = size;

    // Build a string like: float name[size] = float[size](a, b, ... c);
    const_tables += std::format("float {}[{}] = float[{}](", name, size, size);
    for (int i = 0; i < size; i++) {
        const_tables += std::format("{:#g}f", table[i]);
        if (i != size - 1) const_tables += ", ";
    }
    const_tables += ");\n";
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

std::string getSRC(const PS4::GCN::Shader::InstOperand& op) {
    std::string src;
    
    switch (op.field) {
    case OperandField::ScalarGPR:           src = std::format("s[{}]", op.code);                                                        break;
    case OperandField::VectorGPR:           src = std::format("v[{}]", op.code);                                                        break;
    case OperandField::LiteralConst:        src = std::format("{}f", reinterpret_cast<const float&>(op.code));                          break;
    case OperandField::SignedConstIntPos:   src = std::format("intBitsToFloat({})", (s32)op.code - SignedConstIntPosMin + 1);           break;
    case OperandField::SignedConstIntNeg:   src = std::format("intBitsToFloat({})", -(s32)(op.code - SignedConstIntNegMin + 1));        break;
    case OperandField::ConstFloatNeg_4_0:   src = "-4.0f";                                                                              break;
    case OperandField::ConstFloatNeg_2_0:   src = "-2.0f";                                                                              break;
    case OperandField::ConstFloatNeg_1_0:   src = "-1.0f";                                                                              break;
    case OperandField::ConstFloatNeg_0_5:   src = "-0.5f";                                                                              break;
    case OperandField::ConstZero:           src = "0.0f";                                                                               break;
    case OperandField::ConstFloatPos_0_5:   src = "0.5f";                                                                               break;
    case OperandField::ConstFloatPos_1_0:   src = "1.0f";                                                                               break;
    case OperandField::ConstFloatPos_2_0:   src = "2.0f";                                                                               break;
    case OperandField::ConstFloatPos_4_0:   src = "4.0f";                                                                               break;
    case OperandField::ExecLo:              src = "1.0f /* TODO: ExecLo */";                                                            break;
    default:    Helpers::panic("Unhandled SRC %d\n", op.code);
    }

    return src;
}

template<typename T>
T* DescriptorLocation::asPtr() {
    u32 base;
    switch (stage) {
    case ShaderStage::Vertex:   base = Reg::mmSPI_SHADER_USER_DATA_VS_0;    break;
    case ShaderStage::Fragment: base = Reg::mmSPI_SHADER_USER_DATA_PS_0;    break;
    default:    Helpers::panic("DescriptorLocation::asPtr: unhandled shader stage\n");
    }

    if (is_ptr) {
        T* desc;
        std::memcpy(&desc, &GCN::renderer->regs[base + sgpr], sizeof(T*));
        desc = (T*)((u32*)desc + offs);  // The immediate is an offset in dwords
        return desc;
    }
    else {
        return (T*)&GCN::renderer->regs[base + sgpr];
    }
}

template VSharp* DescriptorLocation::asPtr<VSharp>();
template TSharp* DescriptorLocation::asPtr<TSharp>();

void decompileShader(u32* data, ShaderStage stage, ShaderData& out_data, FetchShader* fetch_shader) {
    //std::ofstream out;
    //if (stage == ShaderStage::Fragment) {
    //    out.open("fs.bin", std::ios::binary);
    //    out.write((char*)data, 1_KB);
    //}
    Shader::GcnDecodeContext decoder;
    Shader::GcnCodeSlice code_slice = Shader::GcnCodeSlice((u32*)data, data + std::numeric_limits<u32>::max());

    shader.clear();
    shader.reserve(16_KB);  // Avoid reallocations
    const_tables.clear();
    const_tables.reserve(1_KB);
    const_table_map.clear();
    in_attrs.clear();
    out_attrs.clear();
    in_buffers.clear();

    shader += R"(
#version 430 core
#extension GL_ARB_shading_language_packing : require

float s[104];
float v[104];
vec4 tmp;
bool scc;

)";

    // Parse shader to figure out descriptor locations.
    // This is done similarly to the fetch shader, but there are other cases we need to handle.

    // buf_mapping_idx is used to index buffer_map. We reset it to 0 after parsing is done so that we can reuse it when actually decompiling the instructions.
    // TODO: Beware of this when I implement control flow.... the instructions need to be decompiled in the exact same order they are parsed in below for this to work.
    int buf_mapping_idx = 0;

    // Progressive binding number we use to create new SSBOs
    int next_buf_binding = 0;

    // Map an SGPR to the descriptor location it currently contains
    std::unordered_map<u32, DescriptorLocation> descs;

    // Map a buffer load instruction index (buf_mapping_idx) to buffer ptr
    std::unordered_map<int, Buffer*> buffer_map;

    bool done = false;
    while (!code_slice.atEnd() && !done) {
        const auto instr = decoder.decodeInstruction(code_slice);

        switch (instr.opcode) {
        case Shader::Opcode::S_ENDPGM: {
            done = true;
            continue;
        }

        case Shader::Opcode::S_LOAD_DWORDX4: {
            const auto sgpr_pair = instr.src[0].code * 2;
            const auto dest_pair = instr.dst[0].code;
            descs[dest_pair] = { sgpr_pair, instr.control.smrd.offset };
            break;
        }

        case Shader::Opcode::IMAGE_SAMPLE:
        case Shader::Opcode::S_BUFFER_LOAD_DWORD:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX2:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX4: {
            const auto sgpr = instr.src[2].code * 4;
            if (!descs.contains(sgpr)) {
                // We assume that the descriptor is being passed directly as user data.
                // Check if this buffer already exists, otherwise create a new one
                bool found = false;
                for (auto& buf : out_data.buffers) {
                    if (buf.desc_info.sgpr == sgpr) {
                        // The buffer already exists
                        Helpers::debugAssert(!buf.desc_info.is_ptr, "Error fetching shader descriptor locations");
                        buffer_map[buf_mapping_idx++] = &buf;
                        found = true;
                        break;
                    }
                }

                if (found) break;

                // Create a new one
                auto& buf = out_data.buffers.emplace_back();
                buf.binding = next_buf_binding++;
                if (stage == ShaderStage::Fragment) buf.binding += 16;  // TODO: Not ideal, should probably use different descriptor sets for each shader stage instead.
                buf.desc_info.sgpr = sgpr;
                buf.desc_info.is_ptr = false;
                buf.desc_info.stage = stage;
                switch (instr.inst_class) {
                case InstClass::ScalarMemRd: {
                    buf.desc_info.type = DescriptorType::Vsharp;
                    auto name = std::format("ssbo{}", buf.binding);
                    addInSSBO(name, buf.binding);
                    break;
                }

                case InstClass::VectorMemImgSmp: {
                    buf.desc_info.type = DescriptorType::Tsharp;
                    auto name = std::format("tex{}", buf.binding);
                    addInSampler2D(name, buf.binding);
                    break;
                }

                default: Helpers::panic("ShaderDecompiler: unreachable\n");
                }
                
                buffer_map[buf_mapping_idx++] = &buf;
                //printf("Added buffer %s\n", name.c_str());
            }
            else {
                Helpers::panic("TODO: shader descriptor is not passed directly as user data\n");
            }
            break;
        }

        }
    }

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

    buf_mapping_idx = 0;
    done = false;
    code_slice = Shader::GcnCodeSlice((u32*)data, data + std::numeric_limits<u32>::max());
    while (!code_slice.atEnd() && !done) {
        const auto instr = decoder.decodeInstruction(code_slice);

        switch (instr.opcode) {
        case Shader::Opcode::S_ENDPGM: {
            done = true;
            continue;
        }

        case Shader::Opcode::S_WAITCNT: {
            main += "// S_WAITCNT\n";
            break;
        }

        case Shader::Opcode::S_MOV_B32: {
            if (instr.dst[0].code > 104) {
                main += "// TODO: S_MOV_B32 to special register\n";
                continue;
            }

            main += std::format("s[{}] = {};\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::S_MOV_B64: {
            if (instr.dst[0].code > 104) {
                main += "// TODO: S_MOV_B64 to special register\n";
                continue;
            }

            main += std::format("s[{}] = {};\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::S_WQM_B64: {
            main += "// TODO: S_WQM_B64\n";
            break;
        }

        case Shader::Opcode::S_SWAPPC_B64: {
            main += "// S_SWAPPC_B64\n";
            break;
        }

        case Shader::Opcode::S_CMP_LG_U32: {
            main += std::format("scc = floatBitsToUint({}) != floatBitsToUint({});", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_ADD_F32: {
            main += std::format("v[{}] = {} + {};\n", instr.dst[0].code, getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_SUB_F32: {
            main += std::format("v[{}] = {} - {};\n", instr.dst[0].code, getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_MUL_F32: {
            main += std::format("v[{}] = {} * {};\n", instr.dst[0].code, getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_MAC_F32: {
            main += std::format("v[{}] = {} * {} + {};\n", instr.dst[0].code, getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.dst[0]));
            break;
        }

        case Shader::Opcode::V_CVT_PKRTZ_F16_F32: {
            main += std::format("v[{}] = uintBitsToFloat(packHalf2x16(vec2({}, {})));\n", instr.dst[0].code, getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_MOV_B32: {
            main += std::format("v[{}] = {};\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_CVT_OFF_F32_I4: {
            static constexpr float cvt_table[] = {
                0.0f, 0.0625f, 0.1250f, 0.1875f, 0.2500f, 0.3125f, 0.3750f, 0.4375f,
                -0.5000f, -0.4375f, -0.3750f, -0.3125f, -0.2500f, -0.1875f, -0.1250f, -0.0625f
            };
            addFloatConstTable("cvt_off_f32_i4", (float*)cvt_table, 16);

            main += std::format("v[{}] = cvt_off_f32_i4[floatBitsToInt({}) & 0xf];\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_FRACT_F32: {
            main += std::format("v[{}] = fract({});\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_FLOOR_F32: {
            main += std::format("v[{}] = floor({});\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_RCP_F32: {
            main += std::format("v[{}] = 1.0f / {};\n", instr.dst[0].code, getSRC(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_MED3_F32: {
            const auto src0 = getSRC(instr.src[0]);
            const auto src1 = getSRC(instr.src[1]);
            const auto src2 = getSRC(instr.src[2]);
            // med3(a, b, c) = max(min(a, b), min(max(a, b), c))
            main += std::format("v[{}] = max(min({}, {}), min(max({}, {}), {}));\n", instr.dst[0].code, src0, src1, src0, src1, src2);
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

        case Shader::Opcode::S_BUFFER_LOAD_DWORD: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = instr.control.smrd.imm ? std::format("{}", instr.control.smrd.offset) : std::format("s[{}]", instr.control.smrd.offset);
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code, ssbo_name, offset);
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX2: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX2: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = instr.control.smrd.imm ? std::format("{}", instr.control.smrd.offset) : std::format("s[{}]", instr.control.smrd.offset);
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code, ssbo_name, offset + " + 0");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 1, ssbo_name, offset + " + 1");
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX4: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX4: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = instr.control.smrd.imm ? std::format("{}", instr.control.smrd.offset) : std::format("s[{}]", instr.control.smrd.offset);
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code, ssbo_name, offset + " + 0");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 1, ssbo_name, offset + " + 1");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 2, ssbo_name, offset + " + 2");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 3, ssbo_name, offset + " + 3");
            break;
        }

        case Shader::Opcode::IMAGE_SAMPLE: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_SAMPLE: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto sampler_name = std::format("tex{}", buf->binding);
            const std::string texcoords = std::format("vec2(v[{}], v[{}])", instr.src[0].code, instr.src[0].code + 1);
            main += std::format("tmp = texture({}, {});\n", sampler_name, texcoords);
            main += std::format("v[{}] = tmp.x;\n", instr.dst[0].code + 0);
            main += std::format("v[{}] = tmp.y;\n", instr.dst[0].code + 1);
            main += std::format("v[{}] = tmp.z;\n", instr.dst[0].code + 2);
            main += std::format("v[{}] = tmp.w;\n", instr.dst[0].code + 3);
            break;
        }

        case Shader::Opcode::EXP: {
            const auto tgt = instr.control.exp.target;
            
            // When compr is enabled, EXP uses four 16bit values packed in VSRC0 and VSRC1 instead of four 32bit values in VSRC0, VSRC1, VSRC2 and VSRC3
            std::string data;
            if (!instr.control.exp.compr)
                data = std::format("vec4({}, {}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2]), getSRC(instr.src[3]));
            else
                data = std::format("vec4(unpackHalf2x16(floatBitsToUint({})), unpackHalf2x16(floatBitsToUint({})))", getSRC(instr.src[0]), getSRC(instr.src[1]));

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
            printf("Shader so far:\n%s\n", main.c_str());
            Helpers::panic("Unimplemented shader instruction %d\n", instr.opcode);
            //main += "// TODO\n";
        }
        }
    }

    shader += "\n";
    shader += const_tables;
    shader += "\n";

    // Declare main function
    addFunc("void main", main);

    printf("%s\n", shader.c_str());
    out_data.source = shader;
}

}   // End namespace PS4::GCN::Shader
