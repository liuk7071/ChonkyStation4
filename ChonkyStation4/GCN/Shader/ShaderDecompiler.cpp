#include "ShaderDecompiler.hpp"
#include <GCN/Shader/Decoder.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/DataFormats.hpp>
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

static bool has_cubemap_scoord_func = false;
void addCubemapSCoordFunc() {
    if (has_cubemap_scoord_func) return;
    has_cubemap_scoord_func = true;

    shader += R"(
float scoord(float x, float y, float z) {
    const float abs_x = abs(x);
    const float abs_y = abs(y);
    const float abs_z = abs(z);

    if (abs_z >= abs_x && abs_z >= abs_y) {
        if (z < 0.0f)   return -x;
        else            return x;
    }
    else if (abs_y >= abs_x)
        return x;
    else {
        if (x < 0.0f)   return z;
        else            return -z;
    }
}
)";
}

static bool has_cubemap_tcoord_func = false;
void addCubemapTCoordFunc() {
    if (has_cubemap_tcoord_func) return;
    has_cubemap_tcoord_func = true;

    shader += R"(
float tcoord(float x, float y, float z) {
    const float abs_x = abs(x);
    const float abs_y = abs(y);
    const float abs_z = abs(z);

    if (abs_z >= abs_x && abs_z >= abs_y)
        return -y;
    else if (abs_y >= abs_x) {
        if (y < 0.0f)  return -z;
        else        return z;
    }
    else return -y;
}
)";
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

std::string getType(int n_lanes, u32 nfmt) {
    switch (n_lanes) {
    case 1: {
        switch ((NumberFormat)nfmt) {
        case NumberFormat::Float:   return "float";
        case NumberFormat::Uscaled: return "float";
        default: Helpers::panic("Unhandled n_lanes=%d, nfmt=%d", n_lanes, nfmt);
        }
        break;
    }
    default: {
        switch ((NumberFormat)nfmt) {
        case NumberFormat::Unorm:   return std::format("vec{}", n_lanes);
        case NumberFormat::Snorm:   return std::format("vec{}", n_lanes);
        case NumberFormat::Uscaled: return std::format("vec{}", n_lanes);
        case NumberFormat::Sscaled: return std::format("vec{}", n_lanes);
        case NumberFormat::Float:   return std::format("vec{}", n_lanes);
        default: Helpers::panic("Unhandled n_lanes=%d, nfmt=%d", n_lanes, nfmt);
        }
        break;
    }
    }
}

template<bool is_int = false>
std::string getSRC(const PS4::GCN::Shader::InstOperand& op) {
    std::string src;
    
    switch (op.field) {
    case OperandField::ScalarGPR:           src = std::format("s[{}]", op.code);                                        break;
    case OperandField::VectorGPR:           src = std::format("v[{}]", op.code);                                        break;
    case OperandField::LiteralConst: {
        if constexpr (!is_int)
            src = std::format("f2u({:#g}f)", reinterpret_cast<const float&>(op.code));
        else
            src = std::format("{}", op.code);
        break;
    }
    case OperandField::SignedConstIntPos:   src = std::format("{}", (s32)op.code - SignedConstIntPosMin + 1);           break;
    case OperandField::SignedConstIntNeg:   src = std::format("{}", -(s32)(op.code - SignedConstIntNegMin + 1));        break;
    case OperandField::ConstFloatNeg_4_0:   src = "f2u(-4.0f)";                                                         break;
    case OperandField::ConstFloatNeg_2_0:   src = "f2u(-2.0f)";                                                         break;
    case OperandField::ConstFloatNeg_1_0:   src = "f2u(-1.0f)";                                                         break;
    case OperandField::ConstFloatNeg_0_5:   src = "f2u(-0.5f)";                                                         break;
    case OperandField::ConstZero:           src = "0";                                                                  break;
    case OperandField::ConstFloatPos_0_5:   src = "f2u(0.5f)";                                                          break;
    case OperandField::ConstFloatPos_1_0:   src = "f2u(1.0f)";                                                          break;
    case OperandField::ConstFloatPos_2_0:   src = "f2u(2.0f)";                                                          break;
    case OperandField::ConstFloatPos_4_0:   src = "f2u(4.0f)";                                                          break;
    case OperandField::ExecLo:              src = "f2u(1.0f) /* TODO: ExecLo */";                                       break;
    case OperandField::VccLo:               src = "f2u(1.0f) /* TODO: VccLo */";                                        break;
    case OperandField::VccHi:               src = "f2u(1.0f) /* TODO: VccHi */";                                        break;
    default:    Helpers::panic("Unhandled SRC %d\n", op.code);
    }


    if constexpr (!is_int) {
        src = "u2f(" + src + ")";
        if (op.input_modifier.abs)
            src = "abs(" + src + ")";
        if (op.input_modifier.neg)
            src = "-" + src;
    }

    return src;
}

template<bool is_int = false>
std::string setDST(const PS4::GCN::Shader::InstOperand& op, std::string val) {
    std::string code;
    std::string src;
    src = val;

    if constexpr (!is_int) {
        if (op.output_modifier.multiplier != 0.0f)
            src = std::format("({} * {:#g}f)", src, op.output_modifier.multiplier);
        if (op.output_modifier.clamp)
            src = "clamp(" + src + ", 0.0f, 1.0f)";
        src = "f2u(" + src + ")";
    }

    switch (op.field) {
    case OperandField::ScalarGPR:           code = std::format("s[{}] = {};\n", op.code, src);      break;
    case OperandField::VectorGPR:           code = std::format("v[{}] = {};\n", op.code, src);      break;
    case OperandField::VccLo:               code = "// TODO: Set VccLo\n";                          break;
    case OperandField::VccHi:               code = "// TODO: Set VccHi\n";                          break;
    case OperandField::M0:                  code = "// TODO: Set M0\n";                             break;
    case OperandField::ExecLo:              code = "// TODO: Set ExecLo\n";                         break;
    case OperandField::ExecHi:              code = "// TODO: Set ExecHi\n";                         break;
    default:    Helpers::panic("Unhandled DST %d\n", op.code);
    }

    return code;
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
    std::ofstream out;
    ////if (stage == ShaderStage::Fragment) {
    //    out.open("shader.bin", std::ios::binary);
    //    out.write((char*)data, 1_KB);
    //    out.close();
    ////}
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
    has_cubemap_scoord_func = false;
    has_cubemap_tcoord_func = false;

    shader += R"(
#version 430 core
#extension GL_ARB_shading_language_packing : require

#define u2f(x) uintBitsToFloat(x)
#define f2u(x) floatBitsToUint(x)

vec4 tmp;
float tmp2[4];

uint s[104];
uint v[104];
bool scc;
bool vcc;

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

    // Used to track V_WRITELANE_B32 and V_READLANE_B32.
    // Sometimes the compiler will use these to backup SGPRS.
    std::unordered_map<u32, DescriptorLocation> backup_descs;  // The u32 should correspond to (VGPR << 16) | lane

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

        case Shader::Opcode::S_LOAD_DWORDX4:
        case Shader::Opcode::S_LOAD_DWORDX8: {
            const auto sgpr_pair = instr.src[0].code * 2;
            const auto dest_pair = instr.dst[0].code;
            descs[dest_pair] = { sgpr_pair, instr.control.smrd.offset, true };
            break;
        }

        case Shader::Opcode::V_WRITELANE_B32: {
            const u32 dest_vgpr = instr.dst[0].code;
            const u32 src_sgpr = instr.src[0].code; // TODO: Verify this is an sgpr?
            const u32 lane = std::stoi(getSRC<true>(instr.src[1]));
            if (descs.contains(src_sgpr)) {
                backup_descs[(dest_vgpr << 16) | lane] = descs[src_sgpr];
            }
        }

        case Shader::Opcode::V_READLANE_B32: {
            const u32 dest_sgpr = instr.dst[0].code;
            const u32 src_vgpr = instr.src[0].code; // TODO: Verify this is a vgpr?
            const u32 lane = std::stoi(getSRC<true>(instr.src[1]));
            const u32 backup_idx = (src_vgpr << 16) | lane;
            if (backup_descs.contains(backup_idx)) {
                descs[dest_sgpr] = backup_descs[backup_idx];
            }
            break;
        }

        case Shader::Opcode::IMAGE_SAMPLE:
        case Shader::Opcode::IMAGE_SAMPLE_L:
        case Shader::Opcode::IMAGE_SAMPLE_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ_O:
        case Shader::Opcode::S_BUFFER_LOAD_DWORD:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX2:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX4:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX8:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX16: {
            auto get_buffer = [&](u32 sgpr, bool is_ptr, u32 offs = 0) -> Buffer& {
                // Check if the buffer already exists
                for (auto& buf : out_data.buffers) {
                    if (buf.desc_info.sgpr == sgpr && buf.desc_info.is_ptr == is_ptr && buf.desc_info.offs == offs) {
                        // The buffer already exists
                        return buf;
                    }
                }

                // Create a new one
                auto& buf = out_data.buffers.emplace_back();
                buf.binding = next_buf_binding++;
                if (stage == ShaderStage::Fragment) buf.binding += 16;  // TODO: Not ideal, should probably use different descriptor sets for each shader stage instead.
                buf.desc_info.sgpr = sgpr;
                buf.desc_info.is_ptr = is_ptr;
                buf.desc_info.offs = offs;
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
                    //printf("Created binding for %s\nsgpr=%d, is_ptr=%d, offs=%d\n", name.c_str(), sgpr, is_ptr, offs);
                    break;
                }

                default: Helpers::panic("ShaderDecompiler: unreachable\n");
                }

                return buf;
            };

            const int idx  = instr.inst_class == InstClass::VectorMemImgSmp ? 2 : 0;
            const int mult = instr.inst_class == InstClass::VectorMemImgSmp ? 4 : 2;
            const auto sgpr = instr.src[idx].code * mult;
            if (!descs.contains(sgpr)) {
                // We assume that the descriptor is being passed directly as user data.
                auto& buf = get_buffer(sgpr, false);
                buffer_map[buf_mapping_idx++] = &buf;
                //printf("Found V# in SGPR %d\n", buf.desc_info.sgpr);
                //printf("Added buffer %s\n", name.c_str());
            }
            else {
                auto& desc = descs[sgpr];
                auto& buf = get_buffer(desc.sgpr, desc.is_ptr, desc.offs);
                buffer_map[buf_mapping_idx++] = &buf;
            }
            break;
        }

        }
    }

    std::string main;
    main.reserve(8_KB); // Avoid reallocations

    switch (stage) {
    case ShaderStage::Vertex: {
        // Write vertex inputs from fetch shader
        for (auto& binding : fetch_shader->bindings) {
            VSharp* vsharp = binding.vsharp_loc.asPtr();
            std::string attr = std::format("vs_attr{}", binding.idx);
            addInAttr(attr, getType(binding.n_elements, vsharp->nfmt), binding.idx);

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

                main += std::format("v[{}] = floatBitsToUint({});\n", binding.dest_vgpr + i, val);
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

        case Shader::Opcode::S_CSELECT_B32: {
            main += setDST<true>(instr.dst[0], std::format("scc ? {} : {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            break;
        }
        
        case Shader::Opcode::S_AND_B32: {
            main += setDST<true>(instr.dst[0], std::format("{} & {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            main += std::format("scc = {} != 0;\n", getSRC<true>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_AND_B64: {
            main += setDST<true>(instr.dst[0], std::format("{} & {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            main += std::format("scc = {} != 0;\n", getSRC<true>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_OR_B32: {
            main += setDST<true>(instr.dst[0], std::format("{} | {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            main += std::format("scc = {} != 0;\n", getSRC<true>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_OR_B64: {
            main += setDST<true>(instr.dst[0], std::format("{} | {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            main += std::format("scc = {} != 0;\n", getSRC<true>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_ANDN2_B64: {
            main += setDST<true>(instr.dst[0], std::format("{} & ~{}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            main += std::format("scc = {} != 0;\n", getSRC<true>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_BFE_U32: {
            const auto src0 = getSRC<true>(instr.src[0]);
            const auto src1 = getSRC<true>(instr.src[1]);
            main += setDST<true>(instr.dst[0], std::format("bitfieldExtract({}, {} & 0x1f, bitfieldExtract({}, 16, 7))", src0, src1, src1));
            main += std::format("scc = {} != 0;\n", getSRC<true>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_MOV_B32: {
            main += setDST<true>(instr.dst[0], getSRC<true>(instr.src[0]));
            break;
        }

        case Shader::Opcode::S_MOV_B64: {
            main += setDST<true>(instr.dst[0], getSRC<true>(instr.src[0]));
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

        case Shader::Opcode::S_CBRANCH_EXECZ: {
            main += "// TODO: S_CBRANCH_EXECZ\n";
            break;
        }

        case Shader::Opcode::S_AND_SAVEEXEC_B64: {
            main += "// TODO: S_AND_SAVEEXEC_B64\n";
            break;
        }

        case Shader::Opcode::S_CMP_EQ_U32: {
            main += std::format("scc = {} == {};\n", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LG_U32: {
            main += std::format("scc = {} != {};\n", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_NOP: break;

        case Shader::Opcode::V_CMP_LT_F32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} < {};\n", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_CMP_EQ_F32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} == {};\n", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }
        
        case Shader::Opcode::V_CMP_LE_F32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} <= {};\n", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }
        
        case Shader::Opcode::V_CMP_GT_F32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} > {};\n", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_CMP_GE_F32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} >= {};\n", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_CMP_NEQ_F32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} != {};\n", getSRC(instr.src[0]), getSRC(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CBRANCH_SCC0: {
            main += "// TODO: S_CBRANCH_SCC0\n";
            break;
        }
        
        case Shader::Opcode::S_CBRANCH_VCCZ: {
            main += "// TODO: S_CBRANCH_VCCZ\n";
            break;
        }

        case Shader::Opcode::V_CMP_NE_I32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} != {};\n", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_CMP_EQ_U32: {
            // TODO: This can set other registers too I think?
            main += std::format("vcc = {} == {};\n", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1]));
            break;
        }

        case Shader::Opcode::V_CNDMASK_B32: {
            main += "// TODO: V_CNDMASK_B32\n";
            break;
        }

        case Shader::Opcode::V_ADD_F32: {
            main += setDST(instr.dst[0], std::format("{} + {}", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_SUB_F32: {
            main += setDST(instr.dst[0], std::format("{} - {}", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }
        
        case Shader::Opcode::V_SUBREV_F32: {
            main += setDST(instr.dst[0], std::format("{} - {}", getSRC(instr.src[1]), getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_MUL_LEGACY_F32:
        case Shader::Opcode::V_MUL_F32: {
            main += setDST(instr.dst[0], std::format("{} * {}", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MIN_LEGACY_F32:
        case Shader::Opcode::V_MIN_F32: {
            main += setDST(instr.dst[0], std::format("min({}, {})", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MAX_LEGACY_F32:
        case Shader::Opcode::V_MAX_F32: {
            main += setDST(instr.dst[0], std::format("max({}, {})", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_AND_B32 : {
            main += setDST<true>(instr.dst[0], std::format("{} & {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_OR_B32: {
            main += setDST<true>(instr.dst[0], std::format("{} | {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MAC_LEGACY_F32:
        case Shader::Opcode::V_MAC_F32: {
            main += setDST(instr.dst[0], std::format("{} * {} + {}", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.dst[0])));
            break;
        }

        case Shader::Opcode::V_MADMK_F32: {
            main += setDST(instr.dst[0], std::format("fma({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[2]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_ADD_I32: {
            main += setDST<true>(instr.dst[0], std::format("{} + {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::V_CVT_PKRTZ_F16_F32: {
            main += setDST<true>(instr.dst[0], std::format("packHalf2x16(vec2({}, {}))", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_NOP: break;

        case Shader::Opcode::V_MOV_B32: {
            main += setDST<true>(instr.dst[0], getSRC<true>(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_CVT_F32_I32: {
            // TODO: For now everything is a uint
            main += setDST(instr.dst[0], std::format("float({})", getSRC<true>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_I32_F32: {
            const auto src0 = getSRC(instr.src[0]);

            main += std::format("if      ({} >  {}) tmp.x =  {};\n", src0, INT32_MAX, INT32_MAX);
            main += std::format("else if ({} < -{}) tmp.x = -{};\n", src0, INT32_MAX, INT32_MAX);
            main += std::format("else               tmp.x =  {};\n", src0, INT32_MAX, src0);
            main += setDST<true>(instr.dst[0], "int(tmp.x)");
            break;
        }

        case Shader::Opcode::V_CVT_OFF_F32_I4: {
            static constexpr float cvt_table[] = {
                0.0f, 0.0625f, 0.1250f, 0.1875f, 0.2500f, 0.3125f, 0.3750f, 0.4375f,
                -0.5000f, -0.4375f, -0.3750f, -0.3125f, -0.2500f, -0.1875f, -0.1250f, -0.0625f
            };
            addFloatConstTable("cvt_off_f32_i4", (float*)cvt_table, 16);

            main += setDST(instr.dst[0], std::format("cvt_off_f32_i4[{} & 0xf]", getSRC<true>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_FRACT_F32: {
            main += setDST(instr.dst[0], std::format("fract({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RNDNE_F32: {
            main += setDST(instr.dst[0], std::format("roundEven({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_FLOOR_F32: {
            main += setDST(instr.dst[0], std::format("floor({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_EXP_F32: {
            main += setDST(instr.dst[0], std::format("pow(2.0f, {})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_LOG_F32: {
            main += setDST(instr.dst[0], std::format("log2({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RCP_F32: {
            main += setDST(instr.dst[0], std::format("1.0f / {}", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RSQ_F32: {
            main += setDST(instr.dst[0], std::format("1.0f / sqrt({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_SQRT_F32: {
            main += setDST(instr.dst[0], std::format("sqrt({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_SIN_F32: {
            main += setDST(instr.dst[0], std::format("sin({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_COS_F32: {
            main += setDST(instr.dst[0], std::format("cos({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_MAD_LEGACY_F32:
        case Shader::Opcode::V_MAD_F32: {
            main += setDST(instr.dst[0], std::format("{} * {} + {}", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_CUBESC_F32: {
            addCubemapSCoordFunc();
            main += setDST(instr.dst[0], std::format("scoord({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_CUBETC_F32: {
            addCubemapTCoordFunc();
            main += setDST(instr.dst[0], std::format("tcoord({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MADAK_F32:
        case Shader::Opcode::V_FMA_F32: {
            main += setDST(instr.dst[0], std::format("fma({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MED3_F32: {
            const auto src0 = getSRC(instr.src[0]);
            const auto src1 = getSRC(instr.src[1]);
            const auto src2 = getSRC(instr.src[2]);
            // med3(a, b, c) = max(min(a, b), min(max(a, b), c))
            main += setDST(instr.dst[0], std::format("max(min({}, {}), min(max({}, {}), {}))", src0, src1, src0, src1, src2));
            break;
        }

        case Shader::Opcode::V_MAX_F64: {
            // TODO: This is not right
            main += setDST(instr.dst[0], std::format("max({}, {}) /* V_MAX_F64 */", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MUL_LO_U32: {
            main += setDST<true>(instr.dst[0], std::format("{} * {}", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_INTERP_P1_F32: {
            const std::string attr = std::format("ps_attr{}", instr.control.vintrp.attr);
            addInAttr(attr, "vec4", instr.control.vintrp.attr);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            main += setDST(instr.dst[0], std::format("{}.{}", attr, lanes[instr.control.vintrp.chan]));
            break;
        }

        case Shader::Opcode::V_INTERP_P2_F32: {
            const std::string attr = std::format("ps_attr{}", instr.control.vintrp.attr);
            addInAttr(attr, "vec4", instr.control.vintrp.attr);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            main += setDST(instr.dst[0], std::format("{}.{}", attr, lanes[instr.control.vintrp.chan]));
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX4: {
            main += "// TODO: S_LOAD_DWORDX4\n";
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX8: {
            main += "// TODO: S_LOAD_DWORDX8\n";
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

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX8: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX8: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = instr.control.smrd.imm ? std::format("{}", instr.control.smrd.offset) : std::format("s[{}]", instr.control.smrd.offset);
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code, ssbo_name, offset + " + 0");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 1, ssbo_name, offset + " + 1");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 2, ssbo_name, offset + " + 2");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 3, ssbo_name, offset + " + 3");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 4, ssbo_name, offset + " + 4");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 5, ssbo_name, offset + " + 5");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 6, ssbo_name, offset + " + 6");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 7, ssbo_name, offset + " + 7");
            break;
        }
        
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX16: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX16: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = instr.control.smrd.imm ? std::format("{}", instr.control.smrd.offset) : std::format("s[{}]", instr.control.smrd.offset);
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code, ssbo_name, offset + " + 0");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  1, ssbo_name, offset + " +  1");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  2, ssbo_name, offset + " +  2");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  3, ssbo_name, offset + " +  3");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  4, ssbo_name, offset + " +  4");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  5, ssbo_name, offset + " +  5");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  6, ssbo_name, offset + " +  6");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  7, ssbo_name, offset + " +  7");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  8, ssbo_name, offset + " +  8");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code +  9, ssbo_name, offset + " +  9");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 10, ssbo_name, offset + " + 10");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 11, ssbo_name, offset + " + 11");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 12, ssbo_name, offset + " + 12");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 13, ssbo_name, offset + " + 13");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 14, ssbo_name, offset + " + 14");
            main += std::format("s[{}] = {}.data[{}];\n", instr.dst[0].code + 15, ssbo_name, offset + " + 15");
            break;
        }

        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZW: {
            main += "// TODO: TBUFFER_LOAD_FORMAT_XYZW\n";
            main += std::format("v[{}] = 0;\n", instr.src[1].code);
            main += std::format("v[{}] = 0;\n", instr.src[1].code + 1);
            main += std::format("v[{}] = 0;\n", instr.src[1].code + 2);
            main += std::format("v[{}] = 0;\n", instr.src[1].code + 3);
            break;
        }

        case Shader::Opcode::IMAGE_SAMPLE_L:
        case Shader::Opcode::IMAGE_SAMPLE_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ_O:
            main += "// IMAGE_SAMPLE_*\n";
        case Shader::Opcode::IMAGE_SAMPLE: {
            const auto buffer_mapping = buf_mapping_idx++;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_SAMPLE: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto sampler_name = std::format("tex{}", buf->binding);
            const std::string texcoords = std::format("vec2(u2f(v[{}]), u2f(v[{}]))", instr.src[0].code, instr.src[0].code + 1);
            main += std::format("tmp = texture({}, {});\n", sampler_name, texcoords);
            main += "tmp2 = float[](tmp.x, tmp.y, tmp.z, tmp.w);\n";

            // Set results according to DMASK
            u32 dest_gpr_offs = 0;
            for (int channel = 0; channel < 4; channel++) {
                if (((instr.control.mimg.dmask >> channel) & 1) == 0)
                    continue;

                main += std::format("v[{}] = f2u(tmp2[{}]);\n", instr.dst[0].code + dest_gpr_offs++, channel);
            }
            break;
        }

        case Shader::Opcode::EXP: {
            const auto tgt = instr.control.exp.target;
            
            // When compr is enabled, EXP uses four 16bit values packed in VSRC0 and VSRC1 instead of four 32bit values in VSRC0, VSRC1, VSRC2 and VSRC3
            std::string data;
            if (!instr.control.exp.compr)
                data = std::format("vec4({}, {}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2]), getSRC(instr.src[3]));
            else
                data = std::format("vec4(unpackHalf2x16({}), unpackHalf2x16({}))", getSRC<true>(instr.src[0]), getSRC<true>(instr.src[1]));

            // Color targets
            if (tgt >= 0 && tgt < 8) {
                const std::string attr = std::format("col{}", tgt);
                addOutAttr(attr, "vec4", tgt);
                main += std::format("{} = {};\n", attr, data);
            }
            // Output to Z (TODO)
            else if (tgt == 8) {
                main += "// TODO: Output to Z\n";
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
            //printf("Shader so far:\n%s\n", main.c_str());
            //Helpers::panic("Unimplemented shader instruction %d\n", instr.opcode);
            main += "// TODO\n";
        }
        }
    }

    shader += "\n";
    shader += const_tables;
    shader += "\n";

    // Declare main function
    addFunc("void main", main);

   //printf("%s\n", shader.c_str());
    out_data.source = shader;
}

}   // End namespace PS4::GCN::Shader
