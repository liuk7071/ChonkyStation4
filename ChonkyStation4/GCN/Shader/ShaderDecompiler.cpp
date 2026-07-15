#include "ShaderDecompiler.hpp"
#include <GCN/Shader/Decoder.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/DataFormats.hpp>
#include <GCN/GCN.hpp>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <deque>


namespace PS4::GCN::Shader {

std::string shader;
std::string const_tables;
std::string initialization;
std::unordered_map<std::string, size_t> const_table_map;
std::unordered_map<int, std::string> in_attrs;
std::unordered_map<int, std::string> out_attrs;
std::unordered_map<int, std::string> in_buffers;
std::unordered_map<int, std::string> in_samplers;
std::unordered_map<int, std::string> out_imgs;

struct BasicBlock {
    u32 pc = 0;
    std::string code;
    bool is_end = false;
    
    GcnInst branch_instr;
    BasicBlock* branch_to = nullptr;
    BasicBlock* fallthrough = nullptr;
    std::vector<BasicBlock*> predecessors;

    std::unordered_set<BasicBlock*> post_dominators;
    BasicBlock* immediate_post_dominator = nullptr;
};

void addFunc(std::string name, std::string body) {
    shader += name + "() {\n";
    std::istringstream stream(body);
    for (std::string line; std::getline(stream, line); )
        shader += "\t" + line + "\n";
    shader += "}\n";
}

static bool has_cubemap_id_func = false;
void addCubemapIDFunc() {
    if (has_cubemap_id_func) return;
    has_cubemap_id_func = true;

    shader += R"(
float cubeid(float x, float y, float z) {
    const float abs_x = abs(x);
    const float abs_y = abs(y);
    const float abs_z = abs(z);

    if (abs_z >= abs_x && abs_z >= abs_y) {
        if (z < 0.0f)   return 5.0f;
        else            return 4.0f;
    }
    else if (abs_y >= abs_x) {
        if (y < 0)      return 3.0f;
        else            return 2.0f;
    }
    else {
        if (x < 0.0f)   return 1.0f;
        else            return 0.0f;
    }
}
)";
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

static bool has_cubemap_majoraxis_func = false;
void addCubemapMajorAxisFunc() {
    if (has_cubemap_majoraxis_func) return;
    has_cubemap_majoraxis_func = true;

    shader += R"(
float cubema(float x, float y, float z) {
    const float abs_x = abs(x);
    const float abs_y = abs(y);
    const float abs_z = abs(z);

    if (abs_z >= abs_x && abs_z >= abs_y)
        return z * 2.0f;
    else if (abs_y >= abs_x) {
        return y * 2.0f;
    }
    else return x * 2.0f;
}
)";
}

std::unordered_map<int, bool> has_fetch_buffer_func_map;
void addFetchBufferByteFunc(int binding) {
    if (has_fetch_buffer_func_map.contains(binding)) return;
    has_fetch_buffer_func_map[binding] = true;
    
    const auto ssbo_name = std::format("ssbo{}", binding);
    shader += ""
        "uint fetchBufferByte" + std::to_string(binding) + "(uint offs) {\n"
        "    uint word  = " + ssbo_name + ".data[offs >> 2u];\n"
        "    uint shift = (offs & 3u) << 3u;\n"
        "    return (word >> shift) & 0xffu;\n"
        "}\n";
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
    shader += std::format("layout(binding = {}, std430) buffer {}_t {{ uint data[]; }} {};\n", binding, name, name);
}

void addInSampler(std::string type, std::string name, int binding) {
    if (in_samplers.contains(binding)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(in_samplers[binding] == name, "ShaderDecompiler: tried to add an input sampler2D to an already used binding with a different name\n");
        return;
    }
    in_samplers[binding] = name;
    shader += std::format("layout(binding = {}) uniform {} {};\n", binding, type, name);
}

void addOutImage2D(std::string name, int binding) {
    if (out_imgs.contains(binding)) {
        // Extra check to be safe, this shouldn't ever happen
        Helpers::debugAssert(out_imgs[binding] == name, "ShaderDecompiler: tried to add an output Image2D to an already used binding with a different name\n");
        return;
    }
    out_imgs[binding] = name;
    shader += std::format("layout(binding = {}) uniform writeonly image2D {};\n", binding, name);
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

std::unordered_map<int, bool> vgpr_map;
std::string getVGPR(int n) {
    std::string reg = std::format("v{}", n);
    if (!vgpr_map.contains(n)) {
        vgpr_map[n] = true;
        initialization += std::format("uint {} = 0;\n", reg);
    }
    return reg;
}

std::unordered_map<int, bool> sgpr_map;
std::string getSGPR(int n) {
    std::string reg = std::format("s{}", n);
    if (!sgpr_map.contains(n)) {
        sgpr_map[n] = true;
        initialization += std::format("uint {} = 0;\n", reg);
    }
    return reg;
}

std::unordered_map<u32, bool> lane_map;
std::string getLane(int vgpr, int lane) {
    const u32 backup_idx = (vgpr << 16) | lane;
    const std::string var = std::format("lane{}_{}", vgpr, lane);
    if (!lane_map.contains(backup_idx)) {
        lane_map[backup_idx] = true;
        initialization += std::format("uint {} = 0;\n", var);
    }
    return var;
}

std::string getType(int n_lanes, u32 nfmt) {
    switch (n_lanes) {
    case 1: {
        switch ((NumberFormat)nfmt) {
        case NumberFormat::Unorm:   return "float";
        case NumberFormat::Snorm:   return "float";
        case NumberFormat::Uscaled: return "float";
        case NumberFormat::Sscaled: return "float";
        case NumberFormat::Uint:    return "uint";
        case NumberFormat::Sint:    return "int";
        case NumberFormat::Float:   return "float";
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
        case NumberFormat::Uint:    return std::format("uvec{}", n_lanes);
        case NumberFormat::Sint:    return std::format("ivec{}", n_lanes);
        case NumberFormat::Float:   return std::format("vec{}", n_lanes);
        default: Helpers::panic("Unhandled n_lanes=%d, nfmt=%d", n_lanes, nfmt);
        }
        break;
    }
    }
}

enum class Type {
    Float,
    Uint,
    Int
};

template<Type type = Type::Float>
std::string getSRC(const PS4::GCN::Shader::InstOperand& op) {
    constexpr bool is_float = type == Type::Float;
    std::string src;
    
    switch (op.field) {
    case OperandField::ScalarGPR:           src = getSGPR(op.code);                                                     break;
    case OperandField::VectorGPR:           src = getVGPR(op.code);                                                     break;
    case OperandField::LiteralConst: {
        if constexpr (is_float)
            src = std::format("f2u({:#g}f)", reinterpret_cast<const float&>(op.code));
        else {
            src = std::format("{}", op.code);
            if (type == Type::Uint)
                src += "u";
        }
        break;
    }
    case OperandField::SignedConstIntPos:   src = std::format("{}", (s32)op.code - SignedConstIntPosMin + 1);           break;
    case OperandField::SignedConstIntNeg:   src = std::format("{}", -(s32)(op.code - SignedConstIntNegMin + 1));        break;
    case OperandField::ConstFloatNeg_4_0:   src = "f2u(-4.0f)";                                                         break;
    case OperandField::ConstFloatNeg_2_0:   src = "f2u(-2.0f)";                                                         break;
    case OperandField::ConstFloatNeg_1_0:   src = "f2u(-1.0f)";                                                         break;
    case OperandField::ConstFloatNeg_0_5:   src = "f2u(-0.5f)";                                                         break;
    case OperandField::ConstZero:           src = "0u";                                                                 break;
    case OperandField::ConstFloatPos_0_5:   src = "f2u(0.5f)";                                                          break;
    case OperandField::ConstFloatPos_1_0:   src = "f2u(1.0f)";                                                          break;
    case OperandField::ConstFloatPos_2_0:   src = "f2u(2.0f)";                                                          break;
    case OperandField::ConstFloatPos_4_0:   src = "f2u(4.0f)";                                                          break;
    case OperandField::M0:                  src = "0 /* TODO: M0 */";                                                   break;
    case OperandField::ExecLo:              src = "exec";                                                               break;
    case OperandField::VccLo:               src = "vcc";                                                                break;
    case OperandField::VccHi:               src = "f2u(1.0f) /* TODO: VccHi */";                                        break;
    default:    Helpers::panic("Unhandled SRC %d\n", op.code);
    }

    if constexpr (is_float) {
        src = "u2f(" + src + ")";
        if (op.input_modifier.abs)
            src = "abs(" + src + ")";
        if (op.input_modifier.neg)
            src = "-" + src;
    }

    if constexpr (type == Type::Int)
        src = "int(" + src + ")";

    return src;
}

template<Type type = Type::Float>
std::string setDST(const PS4::GCN::Shader::InstOperand& op, std::string val) {
    constexpr bool is_float = type == Type::Float;
    std::string code;
    std::string src;
    src = val;

    if constexpr (is_float) {
        if (op.output_modifier.multiplier != 0.0f)
            src = std::format("(({}) * {:#g}f)", src, op.output_modifier.multiplier);
        if (op.output_modifier.clamp)
            src = "clamp(" + src + ", 0.0f, 1.0f)";
        src = "f2u(" + src + ")";
    }

    if constexpr (type == Type::Int)
        src = "uint(" + src + ")";

    switch (op.field) {
    case OperandField::ScalarGPR:           code = std::format("{} = {};\n", getSGPR(op.code), src);    break;
    case OperandField::VectorGPR:           code = std::format("{} = {};\n", getVGPR(op.code), src);    break;
    case OperandField::VccLo:               code = std::format("vcc = {};\n", src);                     break;
    case OperandField::VccHi:               code = "// TODO: Set VccHi\n";                              break;
    case OperandField::M0:                  code = "// TODO: Set M0\n";                                 break;
    case OperandField::ExecLo:              code = std::format("exec = {};\n", src);                    break;
    case OperandField::ExecHi:              code = "// TODO: Set ExecHi\n";                             break;
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
    case ShaderStage::Compute:  base = Reg::mmCOMPUTE_USER_DATA_0;          break;
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

template<Type type>
std::string V_CMP(const PS4::GCN::Shader::GcnInst & instr, std::string op) {
    std::string dst;
    if (instr.dst[1].field == OperandField::ScalarGPR) {
        dst = getSGPR(instr.dst[1].code);
    }
    else if (instr.dst[1].field == OperandField::VccLo) {
        dst = "vcc";
    }
    else {
        Helpers::panic("v_cmp_f32: unimplemented operand field");
    }

    auto decompiled = std::format("{} = uint({} {} {});\n", dst, getSRC<type>(instr.src[0]), op, getSRC<type>(instr.src[1]));
    if (instr.IsCmpx()) {
        decompiled += std::format("exec = {};\n", dst);
    }
    return decompiled;
};

// Progressive binding number we use to create new SSBOs
int next_buf_binding = 0;

// Map an SGPR to the descriptor location it currently contains
std::unordered_map<u32, DescriptorLocation> descs;

// Used to track V_WRITELANE_B32 and V_READLANE_B32.
// Sometimes the compiler will use these to backup SGPRS.
std::unordered_map<u32, DescriptorLocation> backup_descs;  // The u32 should correspond to (VGPR << 16) | lane

// Map a buffer load instruction index (buf_mapping_idx) to buffer ptr
std::unordered_map<int, Buffer*> buffer_map;

void trackAndCreateBuffers(ShaderStage stage, ShaderData& out_data, Shader::GcnDecodeContext& decoder, Shader::GcnCodeSlice& code_slice) {
    // Parse shader to figure out descriptor locations.
    // This is done similarly to the fetch shader, but there are other cases we need to handle.

    next_buf_binding = 0;
    descs.clear();
    backup_descs.clear();
    buffer_map.clear();

    u32 pc = 0;
    bool done = false;
    while (!code_slice.atEnd() && !done) {
        const auto instr = decoder.decodeInstruction(code_slice);

        bool is_img_store = false;
        switch (instr.opcode) {
        case Shader::Opcode::S_ENDPGM: {
            done = true;
            continue;
        }

        case Shader::Opcode::S_LOAD_DWORDX4: {
            const auto sgpr_pair = instr.src[0].code * 2;
            const auto dest_pair = instr.dst[0].code;
            descs[dest_pair] = { sgpr_pair, instr.control.smrd.offset, true };
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX8: {
            const auto sgpr_pair = instr.src[0].code * 2;
            const auto dest_pair = instr.dst[0].code;
            descs[dest_pair + 0] = { sgpr_pair, instr.control.smrd.offset + 0u, true };
            descs[dest_pair + 4] = { sgpr_pair, instr.control.smrd.offset + 4u, true };
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX16: {
            const auto sgpr_pair = instr.src[0].code * 2;
            const auto dest_pair = instr.dst[0].code;
            descs[dest_pair + 0] = { sgpr_pair, instr.control.smrd.offset + 0u, true };
            descs[dest_pair + 4] = { sgpr_pair, instr.control.smrd.offset + 4u, true };
            descs[dest_pair + 8] = { sgpr_pair, instr.control.smrd.offset + 8u, true };
            descs[dest_pair + 12] = { sgpr_pair, instr.control.smrd.offset + 12u, true };
            break;
        }

        case Shader::Opcode::V_WRITELANE_B32: {
            const u32 dest_vgpr = instr.dst[0].code;
            const u32 src_sgpr = instr.src[0].code; // TODO: Verify this is an sgpr?
            const u32 lane = std::stoi(getSRC<Type::Uint>(instr.src[1]));
            if (descs.contains(src_sgpr)) {
                backup_descs[(dest_vgpr << 16) | lane] = descs[src_sgpr];
            }
            else {
                backup_descs[(dest_vgpr << 16) | lane] = { src_sgpr, 0, false };
            }
            break;
        }

        case Shader::Opcode::V_READLANE_B32: {
            const u32 dest_sgpr = instr.dst[0].code;
            const u32 src_vgpr = instr.src[0].code; // TODO: Verify this is a vgpr?
            const u32 lane = std::stoi(getSRC<Type::Uint>(instr.src[1]));
            const u32 backup_idx = (src_vgpr << 16) | lane;
            if (backup_descs.contains(backup_idx)) {
                descs[dest_sgpr] = backup_descs[backup_idx];
            }
            break;
        }

        case Shader::Opcode::IMAGE_STORE:
        case Shader::Opcode::IMAGE_ATOMIC_ADD:
        case Shader::Opcode::IMAGE_ATOMIC_UMIN:
            is_img_store = true;
        case Shader::Opcode::IMAGE_GATHER4_LZ:
        case Shader::Opcode::IMAGE_GATHER4_C:
        case Shader::Opcode::IMAGE_LOAD:
        case Shader::Opcode::IMAGE_LOAD_MIP:
        case Shader::Opcode::IMAGE_SAMPLE:
        case Shader::Opcode::IMAGE_SAMPLE_L:
        case Shader::Opcode::IMAGE_SAMPLE_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C:
        case Shader::Opcode::IMAGE_SAMPLE_O:
        case Shader::Opcode::IMAGE_SAMPLE_LZ_O:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ_O:
        case Shader::Opcode::IMAGE_GET_RESINFO:
        case Shader::Opcode::BUFFER_LOAD_DWORD:
        case Shader::Opcode::BUFFER_LOAD_DWORDX2:
        case Shader::Opcode::BUFFER_LOAD_DWORDX3:
        case Shader::Opcode::BUFFER_LOAD_DWORDX4:
        case Shader::Opcode::BUFFER_STORE_DWORD:
        case Shader::Opcode::BUFFER_STORE_DWORDX2:
        case Shader::Opcode::BUFFER_STORE_DWORDX3:
        case Shader::Opcode::BUFFER_STORE_DWORDX4:
        case Shader::Opcode::S_BUFFER_LOAD_DWORD:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX2:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX4:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX8:
        case Shader::Opcode::S_BUFFER_LOAD_DWORDX16:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_X:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XY:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZ:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZW:
        case Shader::Opcode::BUFFER_LOAD_FORMAT_X:
        case Shader::Opcode::BUFFER_LOAD_FORMAT_XY:
        case Shader::Opcode::BUFFER_LOAD_FORMAT_XYZ:
        case Shader::Opcode::BUFFER_LOAD_FORMAT_XYZW: {
            auto get_buffer = [&](u32 sgpr, bool is_ptr, u32 offs, DescriptorType type, bool is_image_store = false) -> Buffer& {
                // Check if the buffer already exists
                for (auto& buf : out_data.buffers) {
                    if (buf.desc_info.sgpr == sgpr && buf.desc_info.is_ptr == is_ptr && buf.desc_info.offs == offs && buf.desc_info.type == type && buf.is_image_store == is_image_store) {
                        // The buffer already exists
                        return buf;
                    }
                }

                // Create a new one
                auto& buf = out_data.buffers.emplace_back();
                buf.binding = next_buf_binding++;
                if (stage == ShaderStage::Fragment) buf.binding += 16;  // TODO: Not ideal, should probably use different descriptor sets for each shader stage instead.

                if (instr.opcode == Shader::Opcode::TBUFFER_LOAD_FORMAT_X
                    || instr.opcode == Shader::Opcode::TBUFFER_LOAD_FORMAT_XY
                    || instr.opcode == Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZ
                    || instr.opcode == Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZW
                    ) {
                    buf.is_instr_typed = true;
                    buf.instr_dfmt = instr.control.mtbuf.dfmt;
                    buf.instr_nfmt = instr.control.mtbuf.nfmt;
                }
                else buf.is_instr_typed = false;

                buf.is_image_store = is_image_store;
                buf.desc_info.sgpr = sgpr;
                buf.desc_info.is_ptr = is_ptr;
                buf.desc_info.offs = offs;
                buf.desc_info.stage = stage;
                switch (instr.inst_class) {
                case InstClass::ScalarMemRd:
                case InstClass::VectorMemBufFmt:
                case InstClass::VectorMemBufNoFmt: {
                    buf.desc_info.type = DescriptorType::Vsharp;
                    auto name = std::format("ssbo{}", buf.binding);
                    addInSSBO(name, buf.binding);
                    //printf("Created binding for %s\nsgpr=%d, is_ptr=%d, offs=%d\n", name.c_str(), sgpr, is_ptr, offs);
                    break;
                }

                case InstClass::VectorMemImgUt:
                case InstClass::VectorMemImgNoSmp:
                case InstClass::VectorMemImgSmp: {
                    buf.desc_info.type = DescriptorType::Tsharp;
                    auto name = std::format("tex{}", buf.binding);

                    switch (instr.opcode) {
                    case Shader::Opcode::IMAGE_ATOMIC_ADD:
                    case Shader::Opcode::IMAGE_ATOMIC_UMIN:
                    case Shader::Opcode::IMAGE_STORE: {
                        buf.is_image_store = true;
                        addOutImage2D(name, buf.binding);
                        break;
                    }
                    default:
                        buf.is_image_store = false;

                        std::string type = "sampler2D";
                        auto* tsharp = buf.desc_info.asPtr<TSharp>();
                        if (tsharp && tsharp->type == 10 /* COLOR 3D */) {
                            type = "sampler3D";
                        }

                        addInSampler(type, name, buf.binding);
                        break;
                    }
                    //printf("Created binding for %s\nsgpr=%d, is_ptr=%d, offs=%d\n", name.c_str(), sgpr, is_ptr, offs);
                    break;
                }

                default: Helpers::panic("ShaderDecompiler: unreachable\n");
                }

                return buf;
                };

            bool is_img = instr.inst_class == InstClass::VectorMemImgSmp || instr.inst_class == InstClass::VectorMemImgNoSmp || instr.inst_class == InstClass::VectorMemImgUt;
            bool is_vector_mem = instr.inst_class == InstClass::VectorMemBufFmt || instr.inst_class == InstClass::VectorMemBufNoFmt;
            const int idx = is_img || is_vector_mem ? 2 : 0;
            const int mult = is_img || is_vector_mem ? 4 : 2;
            const auto sgpr = instr.src[idx].code * mult;
            if (!descs.contains(sgpr)) {
                // We assume that the descriptor is being passed directly as user data.
                auto& buf = get_buffer(sgpr, false, 0, is_img ? DescriptorType::Tsharp : DescriptorType::Vsharp, is_img_store);
                buffer_map[pc] = &buf;
                //printf("Found V# in SGPR %d\n", buf.desc_info.sgpr);
            }
            else {
                auto& desc = descs[sgpr];
                auto& buf = get_buffer(desc.sgpr, desc.is_ptr, desc.offs, is_img ? DescriptorType::Tsharp : DescriptorType::Vsharp, is_img_store);
                buffer_map[pc] = &buf;
            }
            break;
        }

        }

        pc += instr.length;
    }
}

std::vector<std::unique_ptr<BasicBlock>> blocks;
std::unordered_map<u32, BasicBlock*> block_map;
std::unordered_set<u32> block_entries;

void decompileBasicBlock(u32* data, u32 start_pc, ShaderStage stage, BasicBlock& block);
BasicBlock* getOrCreateBlock(u32* data, u32 pc, ShaderStage stage) {
    // Return the existing block, if any
    if (block_map.contains(pc))
        return block_map[pc];

    auto* block = blocks.emplace_back(std::make_unique<BasicBlock>()).get();
    block->pc = pc;
    block_map[pc] = block;
    decompileBasicBlock(data, pc, stage, *block);
    return block;
}

void discoverBasicBlockEntries(u32* data, u32 pc) {
    if (!block_entries.insert(pc).second)
        return;

    Shader::GcnDecodeContext decoder;
    Shader::GcnCodeSlice code_slice = Shader::GcnCodeSlice((u32*)((u8*)data + pc), data + std::numeric_limits<u32>::max());

    bool done = false;
    while (!code_slice.atEnd() && !done) {
        const auto instr = decoder.decodeInstruction(code_slice);

        switch (instr.opcode) {
        case Shader::Opcode::S_ENDPGM: {
            done = true;
            continue;
        }
        }

        if (instr.IsConditionalBranch()) {
            discoverBasicBlockEntries(data, instr.BranchTarget(pc));
            discoverBasicBlockEntries(data, pc + instr.length);
            return;
        }
        else if (instr.IsUnconditionalBranch()) {
            discoverBasicBlockEntries(data, instr.BranchTarget(pc));
            return;
        }

        pc += instr.length;
    }
}

void decompileBasicBlock(u32* data, u32 start_pc, ShaderStage stage, BasicBlock& block) {
    Shader::GcnDecodeContext decoder;
    Shader::GcnCodeSlice code_slice = Shader::GcnCodeSlice((u32*)((u8*)data + start_pc), data + std::numeric_limits<u32>::max());

    auto s_buffer_load_dword_offset = [&](const PS4::GCN::Shader::GcnInst& instr) -> std::string {
        if (instr.control.smrd.imm)
            return std::format("{}", instr.control.smrd.offset);
        else if (instr.control.smrd.offset == (u32)OperandField::LiteralConst)
            return std::format("{}", instr.src[1].code);

        // Thanks shadPS4, this is not mentioned anywhere in the docs...?
        return std::format("({} >> 2u)", getSGPR(instr.control.smrd.offset));
    };

    auto& code = block.code;
    code.reserve(128_KB);

    code += std::format("// BASIC BLOCK {:08x}\n", start_pc);

    u32 pc = start_pc;
    bool done = false;
    while (!code_slice.atEnd() && !done) {
        // If the current PC is the start of another block, stop.
        if (block_entries.contains(pc) && pc != block.pc) {
            block.fallthrough = getOrCreateBlock(data, pc, stage);
            done = true;
            continue;
        }

        const auto instr = decoder.decodeInstruction(code_slice);

        code += std::format("/* {:08x} */ ", pc);

        switch (instr.opcode) {
        case Shader::Opcode::S_ENDPGM: {
            block.is_end = true;
            done = true;
            continue;
        }

        case Shader::Opcode::S_WAITCNT: {
            code += "// S_WAITCNT\n";
            break;
        }

        case Shader::Opcode::S_ADD_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} + {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::S_SUB_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} - {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            // TODO: Borrow
            break;
        }

        case Shader::Opcode::S_ADD_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("{} + {}", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::S_SUB_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("{} - {}", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            // TODO: Borrow
            break;
        }

        case Shader::Opcode::S_ADDC_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} + {} + scc", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::S_MIN_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("min({}, {})", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} == {});\n", getSRC<Type::Uint>(instr.dst[0]), getSRC<Type::Uint>(instr.src[0]));
            break;
        }

        case Shader::Opcode::S_CSELECT_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("(scc == 1) ? {} : {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::S_CSELECT_B64: {
            // TODO: 64bit
            code += setDST<Type::Uint>(instr.dst[0], std::format("(scc == 1) ? {} : {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::S_AND_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} & {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_AND_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} & {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_OR_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} | {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_OR_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} | {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_XOR_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} ^ {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_ANDN2_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} & ~{}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_NAND_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("~({} & {})", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_NOR_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("~({} | {})", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_LSHL_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} << ({} & 0x1f)", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_LSHR_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} >> ({} & 0x1f)", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_BFM_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("((1 << ({} & 0x1f)) - 1) << ({} & 0x1f)", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::S_MUL_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("{} * {}", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::S_BFE_U32: {
            const auto src0 = getSRC<Type::Uint>(instr.src[0]);
            const auto src1 = getSRC<Type::Int>(instr.src[1]);
            code += setDST<Type::Uint>(instr.dst[0], std::format("bitfieldExtract({}, {} & 0x1f, bitfieldExtract({}, 16, 7))", src0, src1, src1));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_MOVK_I32: {
            const s16 imm16 = instr.control.sopk.simm;
            const s32 imm32 = (s32)imm16;
            code += setDST<Type::Int>(instr.dst[0], std::format("{}", imm32));
            break;
        }

        case Shader::Opcode::S_CMPK_GT_U32: {
            code += "// TODO: S_CMPK_GT_U32\n";
            break;
        }

        case Shader::Opcode::S_ADDK_I32: {
            const s16 imm16 = instr.control.sopk.simm;
            const s32 imm32 = (s32)imm16;
            code += setDST<Type::Int>(instr.dst[0], std::format("{} + {}", getSRC<Type::Int>(instr.dst[0]), imm32));
            break;
        }

        case Shader::Opcode::S_MOV_B32: {
            code += setDST<Type::Uint>(instr.dst[0], getSRC<Type::Uint>(instr.src[0]));
            break;
        }

        case Shader::Opcode::S_MOV_B64: {
            code += setDST<Type::Uint>(instr.dst[0], getSRC<Type::Uint>(instr.src[0]));
            break;
        }

        case Shader::Opcode::S_NOT_B64: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("~{}", getSRC<Type::Uint>(instr.src[0])));
            code += std::format("scc = uint({} != 0);\n", getSRC<Type::Uint>(instr.dst[0]));
            break;
        }

        case Shader::Opcode::S_WQM_B64: {
            code += "// TODO: S_WQM_B64\n";
            break;
        }

        case Shader::Opcode::S_BREV_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("bitfieldReverse({})", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::S_GETPC_B64: {
            code += "// TODO: S_GETPC_B64\n";
            break;
        }

        case Shader::Opcode::S_SETPC_B64: {
            code += "// TODO: S_SETPC_B64\n";
            break;
        }

        case Shader::Opcode::S_SWAPPC_B64: {
            code += std::format("// S_SWAPPC_B64 {}\n", instr.src[0].code);
            break;
        }

        case Shader::Opcode::S_CBRANCH_EXECZ: {
            const auto branch_target = instr.BranchTarget(pc);
            code += std::format("// S_CBRANCH_EXECZ {:08x}\n", branch_target);
            
            block.branch_instr  = instr;
            block.branch_to     = getOrCreateBlock(data, branch_target, stage);
            block.fallthrough   = getOrCreateBlock(data, pc + instr.length, stage);
            done = true;
            continue;
        }

        case Shader::Opcode::S_AND_SAVEEXEC_B64: {
            code += setDST<Type::Uint>(instr.dst[0], "exec");
            code += std::format("exec = {} & exec;\n", getSRC<Type::Uint>(instr.src[0]));
            code += "scc = uint(exec != 0);\n";
            break;
        }

        case Shader::Opcode::S_CMP_EQ_I32: {
            code += std::format("scc = uint({} == {});\n", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LG_I32: {
            code += std::format("scc = uint({} != {});\n", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_GT_I32: {
            code += std::format("scc = uint({} > {});\n", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_GE_I32: {
            code += std::format("scc = uint({} >= {});\n", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LT_I32: {
            code += std::format("scc = uint({} < {});\n", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LE_I32: {
            code += std::format("scc = uint({} <= {});\n", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_EQ_U32: {
            code += std::format("scc = uint({} == {});\n", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LG_U32: {
            code += std::format("scc = uint({} != {});\n", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_GT_U32: {
            code += std::format("scc = uint({} > {});\n", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_GE_U32: {
            code += std::format("scc = uint({} >= {});\n", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LT_U32: {
            code += std::format("scc = uint({} < {});\n", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_CMP_LE_U32: {
            code += std::format("scc = uint({} <= {});\n", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));
            break;
        }

        case Shader::Opcode::S_BRANCH: {
            const auto branch_target = instr.BranchTarget(pc);
            code += std::format("// S_BRANCH {:08x}\n", branch_target);

            block.branch_instr = instr;
            block.branch_to = getOrCreateBlock(data, branch_target, stage);
            block.fallthrough = nullptr;
            done = true;
            continue;
        }

        case Shader::Opcode::S_NOP: break;

        case Shader::Opcode::S_CBRANCH_VCCNZ: {
            const auto branch_target = instr.BranchTarget(pc);
            code += std::format("// S_CBRANCH_VCCNZ {:08x}\n", branch_target);

            block.branch_instr = instr;
            block.branch_to = getOrCreateBlock(data, branch_target, stage);
            block.fallthrough = getOrCreateBlock(data, pc + instr.length, stage);
            done = true;
            continue;
        }

        case Shader::Opcode::S_BARRIER: {
            code += "barrier();\n";
            break;
        }

        case Shader::Opcode::S_TTRACEDATA: {
            code += "// S_TTRACEDATA\n";
            break;
        }

        case Shader::Opcode::V_CMP_NGE_F32:
        case Shader::Opcode::V_CMP_LT_F32:      code += V_CMP<Type::Float>(instr, "<");     break;
        case Shader::Opcode::V_CMP_NLG_F32:
        case Shader::Opcode::V_CMP_EQ_F32:      code += V_CMP<Type::Float>(instr, "==");    break;
        case Shader::Opcode::V_CMP_NGT_F32:
        case Shader::Opcode::V_CMP_LE_F32:      code += V_CMP<Type::Float>(instr, "<=");    break;
        case Shader::Opcode::V_CMP_NLE_F32:
        case Shader::Opcode::V_CMP_GT_F32:      code += V_CMP<Type::Float>(instr, ">");     break;
        case Shader::Opcode::V_CMP_NLT_F32:
        case Shader::Opcode::V_CMP_GE_F32:      code += V_CMP<Type::Float>(instr, ">=");    break;
        case Shader::Opcode::V_CMP_LG_F32:
        case Shader::Opcode::V_CMP_NEQ_F32:     code += V_CMP<Type::Float>(instr, "!=");    break;

        case Shader::Opcode::S_CBRANCH_SCC0: {
            const auto branch_target = instr.BranchTarget(pc);
            code += std::format("// S_CBRANCH_SCC0 {:08x}\n", branch_target);

            block.branch_instr = instr;
            block.branch_to = getOrCreateBlock(data, branch_target, stage);
            block.fallthrough = getOrCreateBlock(data, pc + instr.length, stage);
            done = true;
            continue;
        }

        case Shader::Opcode::S_CBRANCH_SCC1: {
            const auto branch_target = instr.BranchTarget(pc);
            code += std::format("// S_CBRANCH_SCC1 {:08x}\n", branch_target);

            block.branch_instr = instr;
            block.branch_to = getOrCreateBlock(data, branch_target, stage);
            block.fallthrough = getOrCreateBlock(data, pc + instr.length, stage);
            done = true;
            continue;
        }

        case Shader::Opcode::S_CBRANCH_VCCZ: {
            const auto branch_target = instr.BranchTarget(pc);
            code += std::format("// S_CBRANCH_VCCZ {:08x}\n", branch_target);

            block.branch_instr = instr;
            block.branch_to = getOrCreateBlock(data, branch_target, stage);
            block.fallthrough = getOrCreateBlock(data, pc + instr.length, stage);
            done = true;
            continue;
        }

        case Shader::Opcode::V_CMPX_NGE_F32:
        case Shader::Opcode::V_CMPX_LT_F32:     code += V_CMP<Type::Float>(instr, "<");     break;
        case Shader::Opcode::V_CMPX_NLG_F32:
        case Shader::Opcode::V_CMPX_EQ_F32:     code += V_CMP<Type::Float>(instr, "==");    break;
        case Shader::Opcode::V_CMPX_NGT_F32:
        case Shader::Opcode::V_CMPX_LE_F32:     code += V_CMP<Type::Float>(instr, "<=");    break;
        case Shader::Opcode::V_CMPX_NLE_F32:
        case Shader::Opcode::V_CMPX_GT_F32:     code += V_CMP<Type::Float>(instr, ">");     break;
        case Shader::Opcode::V_CMPX_NLT_F32:
        case Shader::Opcode::V_CMPX_GE_F32:     code += V_CMP<Type::Float>(instr, ">=");    break;
        case Shader::Opcode::V_CMPX_LG_F32:
        case Shader::Opcode::V_CMPX_NEQ_F32:    code += V_CMP<Type::Float>(instr, "!=");    break;

        case Shader::Opcode::V_CMP_LT_I32:      code += V_CMP<Type::Int>(instr, "<");       break;
        case Shader::Opcode::V_CMP_EQ_I32:      code += V_CMP<Type::Int>(instr, "==");      break;
        case Shader::Opcode::V_CMP_LE_I32:      code += V_CMP<Type::Int>(instr, "<=");      break;
        case Shader::Opcode::V_CMP_GT_I32:      code += V_CMP<Type::Int>(instr, ">");       break;
        case Shader::Opcode::V_CMP_GE_I32:      code += V_CMP<Type::Int>(instr, ">=");      break;
        case Shader::Opcode::V_CMP_NE_I32:      code += V_CMP<Type::Int>(instr, "!=");      break;

        case Shader::Opcode::V_CMP_LT_U32:      code += V_CMP<Type::Uint>(instr, "<");      break;
        case Shader::Opcode::V_CMP_EQ_U32:      code += V_CMP<Type::Uint>(instr, "==");     break;
        case Shader::Opcode::V_CMP_LE_U32:      code += V_CMP<Type::Uint>(instr, "<=");     break;
        case Shader::Opcode::V_CMP_GT_U32:      code += V_CMP<Type::Uint>(instr, ">");      break;
        case Shader::Opcode::V_CMP_GE_U32:      code += V_CMP<Type::Uint>(instr, ">=");     break;
        case Shader::Opcode::V_CMP_NE_U32:      code += V_CMP<Type::Uint>(instr, "!=");     break;

        case Shader::Opcode::V_CMPX_LT_U32:     code += V_CMP<Type::Uint>(instr, "<");      break;
        case Shader::Opcode::V_CMPX_EQ_U32:     code += V_CMP<Type::Uint>(instr, "==");     break;
        case Shader::Opcode::V_CMPX_LE_U32:     code += V_CMP<Type::Uint>(instr, "<=");     break;
        case Shader::Opcode::V_CMPX_GT_U32:     code += V_CMP<Type::Uint>(instr, ">");      break;
        case Shader::Opcode::V_CMPX_GE_U32:     code += V_CMP<Type::Uint>(instr, ">=");     break;
        case Shader::Opcode::V_CMPX_NE_U32:     code += V_CMP<Type::Uint>(instr, "!=");     break;

        case Shader::Opcode::V_CNDMASK_B32: {
            std::string cond;
            if (instr.src[2].field == OperandField::ScalarGPR) {
                cond = getSGPR(instr.src[2].code);
            }
            else {
                cond = "vcc";
            }

            code += setDST<Type::Uint>(instr.dst[0], std::format("({} == 1) ? {} : {}", cond, getSRC<Type::Uint>(instr.src[1]), getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_READFIRSTLANE_B32: {
            code += "// TODO: V_READFIRSTLANE_B32\n";
            break;
        }

        case Shader::Opcode::V_READLANE_B32: {
            const u32 dest_sgpr = instr.dst[0].code;
            const u32 src_vgpr = instr.src[0].code; // TODO: Verify this is a vgpr?
            const u32 lane = std::stoi(getSRC<Type::Uint>(instr.src[1]));
            code += "// TODO: V_READLANE_B32 ";
            code += std::format("dest: {} src: {} lane: {}\n", dest_sgpr, src_vgpr, lane);
            code += std::format("{} = {};\n", getSGPR(dest_sgpr), getLane(src_vgpr, lane));
            break;
        }

        case Shader::Opcode::V_WRITELANE_B32: {
            const u32 dest_vgpr = instr.dst[0].code;
            const u32 src_sgpr = instr.src[0].code; // TODO: Verify this is an sgpr?
            const u32 lane = std::stoi(getSRC<Type::Uint>(instr.src[1]));
            code += "// TODO: V_WRITELANE_B32 ";
            code += std::format("dest: {} src: {} lane: {}\n", dest_vgpr, src_sgpr, lane);
            code += std::format("{} = {};\n", getLane(dest_vgpr, lane), getSGPR(src_sgpr));
            break;
        }

        case Shader::Opcode::V_ADD_F32: {
            code += setDST(instr.dst[0], std::format("{} + {}", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_SUB_F32: {
            code += setDST(instr.dst[0], std::format("{} - {}", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_SUBREV_F32: {
            code += setDST(instr.dst[0], std::format("{} - {}", getSRC(instr.src[1]), getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_MUL_LEGACY_F32:
        case Shader::Opcode::V_MUL_F32: {
            code += setDST(instr.dst[0], std::format("{} * {}", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MUL_I32_I24: {
            code += setDST<Type::Int>(instr.dst[0], std::format("({} & 0xffffff) * ({} & 0xffffff)", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MUL_U32_U24: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("({} & 0xffffff) * ({} & 0xffffff)", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MIN_LEGACY_F32:
        case Shader::Opcode::V_MIN_F32: {
            code += setDST(instr.dst[0], std::format("min({}, {})", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MAX_LEGACY_F32:
        case Shader::Opcode::V_MAX_F32: {
            code += setDST(instr.dst[0], std::format("max({}, {})", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MIN_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("min({}, {})", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MAX_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("max({}, {})", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MIN_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("min({}, {})", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MAX_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("max({}, {})", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_LSHRREV_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} >> ({} & 0x1f)", getSRC<Type::Uint>(instr.src[1]), getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_ASHRREV_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("{} >> ({} & 0x1f)", getSRC<Type::Int>(instr.src[1]), getSRC<Type::Int>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_LSHLREV_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} << ({} & 0x1f)", getSRC<Type::Uint>(instr.src[1]), getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_AND_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} & {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_OR_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} | {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_XOR_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} ^ {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MAC_LEGACY_F32:
        case Shader::Opcode::V_MAC_F32: {
            code += setDST(instr.dst[0], std::format("{} * {} + {}", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.dst[0])));
            break;
        }

        case Shader::Opcode::V_MADMK_F32: {
            code += setDST(instr.dst[0], std::format("fma({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[2]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_BCNT_U32_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("bitCount({}) + {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_ADD_I32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} + {}", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::V_SUB_I32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} - {}", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            // TODO: Carry out
            break;
        }

        case Shader::Opcode::V_LDEXP_F32: {
            code += setDST<Type::Float>(instr.dst[0], std::format("ldexp({}, {})", getSRC<Type::Float>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_CVT_PKRTZ_F16_F32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("packHalf2x16(vec2({}, {}))", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_NOP: break;

        case Shader::Opcode::V_MOV_B32: {
            code += setDST<Type::Uint>(instr.dst[0], getSRC<Type::Uint>(instr.src[0]));
            break;
        }

        case Shader::Opcode::V_CVT_F32_I32: {
            code += setDST(instr.dst[0], std::format("float({})", getSRC<Type::Int>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_F32_U32: {
            code += setDST(instr.dst[0], std::format("float({})", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_U32_F32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("uint({})", getSRC<Type::Float>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_I32_F32: {
            const auto src0 = getSRC(instr.src[0]);

            code += std::format("if      ({} >  {}) tmp.x =  {};\n", src0, INT32_MAX, INT32_MAX);
            code += std::format("else if ({} < -{}) tmp.x = -{};\n", src0, INT32_MAX, INT32_MAX);
            code += std::format("else               tmp.x =  {};\n", src0);
            code += setDST<Type::Uint>(instr.dst[0], "int(tmp.x)");
            break;
        }

        case Shader::Opcode::V_CVT_F16_F32: {
            code += setDST<Type::Float>(instr.dst[0], std::format("(packHalf2x16(vec2({}, 0.0f)) & 0xffffu)", getSRC<Type::Float>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_F32_F16: {
            code += setDST<Type::Float>(instr.dst[0], std::format("unpackHalf2x16({}).x", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_FLR_I32_F32: {
            const auto src0 = getSRC(instr.src[0]);

            code += std::format("if      ({} >  {}) tmp.x =  {};\n", src0, INT32_MAX, INT32_MAX);
            code += std::format("else if ({} < -{}) tmp.x = -{};\n", src0, INT32_MAX, INT32_MAX);
            code += std::format("else               tmp.x =  {};\n", src0);
            code += setDST<Type::Uint>(instr.dst[0], "int(floor(tmp.x))");
            break;
        }

        case Shader::Opcode::V_CVT_OFF_F32_I4: {
            static constexpr float cvt_table[] = {
                0.0f, 0.0625f, 0.1250f, 0.1875f, 0.2500f, 0.3125f, 0.3750f, 0.4375f,
                -0.5000f, -0.4375f, -0.3750f, -0.3125f, -0.2500f, -0.1875f, -0.1250f, -0.0625f
            };
            addFloatConstTable("cvt_off_f32_i4", (float*)cvt_table, 16);

            code += setDST(instr.dst[0], std::format("cvt_off_f32_i4[{} & 0xf]", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_F32_UBYTE0: {
            code += setDST(instr.dst[0], std::format("float({} & 0xff)", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_F32_UBYTE1: {
            code += setDST(instr.dst[0], std::format("float(({} >> 8) & 0xff)", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_F32_UBYTE2: {
            code += setDST(instr.dst[0], std::format("float(({} >> 16) & 0xff)", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CVT_F32_UBYTE3: {
            code += setDST(instr.dst[0], std::format("float(({} >> 24) & 0xff)", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_FRACT_F32: {
            code += setDST(instr.dst[0], std::format("fract({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_TRUNC_F32: {
            code += setDST(instr.dst[0], std::format("trunc({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_CEIL_F32: {
            // TODO: The manual says it's actually implemented as trunc...?
            code += setDST(instr.dst[0], std::format("ceil({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RNDNE_F32: {
            code += setDST(instr.dst[0], std::format("roundEven({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_FLOOR_F32: {
            code += setDST(instr.dst[0], std::format("floor({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_EXP_F32: {
            code += setDST(instr.dst[0], std::format("exp2({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_LOG_F32: {
            code += setDST(instr.dst[0], std::format("log2({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RCP_F32: {
            code += setDST(instr.dst[0], std::format("1.0f / {}", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RSQ_CLAMP_F32: {
            code += setDST(instr.dst[0], std::format("clamp(1.0f / sqrt({}), -3.402823466e+38f, +3.402823466e+38f)", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_RSQ_F32: {
            code += setDST(instr.dst[0], std::format("1.0f / sqrt({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_SQRT_F32: {
            code += setDST(instr.dst[0], std::format("sqrt({})", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_SIN_F32: {
            code += setDST(instr.dst[0], std::format("sin({} * 2 * PI)", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_COS_F32: {
            code += setDST(instr.dst[0], std::format("cos({} * 2 * PI)", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_NOT_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("~{}", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_BFREV_B32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("bitfieldReverse({})", getSRC<Type::Uint>(instr.src[0])));
            break;
        }

        case Shader::Opcode::V_MAD_LEGACY_F32:
        case Shader::Opcode::V_MAD_F32: {
            code += setDST(instr.dst[0], std::format("{} * {} + {}", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MAD_U32_U24: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("({} & 0xffffff) * ({} & 0xffffff) + {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]), getSRC<Type::Uint>(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_CUBEID_F32: {
            addCubemapIDFunc();
            code += setDST(instr.dst[0], std::format("cubeid({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_CUBESC_F32: {
            addCubemapSCoordFunc();
            code += setDST(instr.dst[0], std::format("scoord({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_CUBETC_F32: {
            addCubemapTCoordFunc();
            code += setDST(instr.dst[0], std::format("tcoord({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_CUBEMA_F32: {
            addCubemapMajorAxisFunc();
            code += setDST(instr.dst[0], std::format("cubema({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_BFE_U32: {
            const auto src0 = getSRC<Type::Uint>(instr.src[0]);
            const auto src1 = getSRC<Type::Uint>(instr.src[1]);
            const auto src2 = getSRC<Type::Uint>(instr.src[2]);
            code += setDST<Type::Uint>(instr.dst[0], std::format("bitfieldExtract({}, int({} & 0x1f), int({} & 0x1f))", src0, src1, src2));
            break;
        }

        case Shader::Opcode::V_BFE_I32: {
            const auto src0 = getSRC<Type::Int>(instr.src[0]);
            const auto src1 = getSRC<Type::Int>(instr.src[1]);
            const auto src2 = getSRC<Type::Int>(instr.src[2]);
            code += setDST<Type::Int>(instr.dst[0], std::format("bitfieldExtract({}, int({} & 0x1f), int({} & 0x1f))", src0, src1, src2));
            break;
        }

        case Shader::Opcode::V_BFI_B32: {
            const auto src0 = getSRC<Type::Uint>(instr.src[0]);
            const auto src1 = getSRC<Type::Uint>(instr.src[1]);
            const auto src2 = getSRC<Type::Uint>(instr.src[2]);
            code += setDST<Type::Uint>(instr.dst[0], std::format("({} & {}) | (~{} & {})", src0, src1, src0, src2));
            break;
        }

        case Shader::Opcode::V_MADAK_F32:
        case Shader::Opcode::V_FMA_F32: {
            code += setDST(instr.dst[0], std::format("fma({}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MIN3_F32: {
            code += setDST(instr.dst[0], std::format("min(min({}, {}), {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MAX3_F32: {
            code += setDST(instr.dst[0], std::format("max(max({}, {}), {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MED3_F32: {
            const auto src0 = getSRC(instr.src[0]);
            const auto src1 = getSRC(instr.src[1]);
            const auto src2 = getSRC(instr.src[2]);
            // med3(a, b, c) = max(min(a, b), min(max(a, b), c))
            code += setDST(instr.dst[0], std::format("max(min({}, {}), min(max({}, {}), {}))", src0, src1, src0, src1, src2));
            break;
        }

        case Shader::Opcode::V_SAD_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("uint(abs({} - {})) + {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]), getSRC<Type::Uint>(instr.src[2])));
            break;
        }

        case Shader::Opcode::V_MAX_F64: {
            // TODO: This is not right
            code += setDST(instr.dst[0], std::format("max({}, {}) /* V_MAX_F64 */", getSRC(instr.src[0]), getSRC(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MUL_LO_U32: {
            code += setDST<Type::Uint>(instr.dst[0], std::format("{} * {}", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_MUL_LO_I32: {
            code += setDST<Type::Int>(instr.dst[0], std::format("{} * {}", getSRC<Type::Int>(instr.src[0]), getSRC<Type::Int>(instr.src[1])));
            break;
        }

        case Shader::Opcode::V_INTERP_P1_F32: {
            const auto attr_idx = instr.control.vintrp.attr;
            const auto location = GCN::renderer->regs[Reg::mmSPI_PS_INPUT_CNTL_0 + attr_idx] & 0x1f;
            const std::string attr = std::format("ps_attr{}", attr_idx);
            addInAttr(attr, "vec4", location);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            code += setDST(instr.dst[0], std::format("{}.{}", attr, lanes[instr.control.vintrp.chan]));
            break;
        }

        case Shader::Opcode::V_INTERP_P2_F32: {
            const auto attr_idx = instr.control.vintrp.attr;
            const auto location = GCN::renderer->regs[Reg::mmSPI_PS_INPUT_CNTL_0 + attr_idx] & 0x1f;
            const std::string attr = std::format("ps_attr{}", attr_idx);
            addInAttr(attr, "vec4", location);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            code += setDST(instr.dst[0], std::format("{}.{}", attr, lanes[instr.control.vintrp.chan]));
            break;
        }

        case Shader::Opcode::V_INTERP_MOV_F32: {
            const auto attr_idx = instr.control.vintrp.attr;
            const auto location = GCN::renderer->regs[Reg::mmSPI_PS_INPUT_CNTL_0 + attr_idx] & 0x1f;
            const std::string attr = std::format("ps_attr{}", attr_idx);
            addInAttr(attr, "vec4", location);
            char lanes[4] = { 'x', 'y', 'z', 'w' };
            code += setDST(instr.dst[0], std::format("{}.{}", attr, lanes[instr.control.vintrp.chan]));
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX4: {
            code += "// TODO: S_LOAD_DWORDX4 dest: " + std::to_string(instr.dst[0].code) + " offs: " + std::to_string(instr.control.smrd.offset) + "\n";
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX8: {
            code += "// TODO: S_LOAD_DWORDX8 dest: " + std::to_string(instr.dst[0].code) + " offs: " + std::to_string(instr.control.smrd.offset) + "\n";
            break;
        }

        case Shader::Opcode::S_LOAD_DWORDX16: {
            code += "// TODO: S_LOAD_DWORDX16 dest: " + std::to_string(instr.dst[0].code) + " offs: " + std::to_string(instr.control.smrd.offset) + "\n";
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORD: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = s_buffer_load_dword_offset(instr);
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code), ssbo_name, offset);
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX2: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX2: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = s_buffer_load_dword_offset(instr);
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 0), ssbo_name, offset + " + 0");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 1), ssbo_name, offset + " + 1");
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX4: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX4: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = s_buffer_load_dword_offset(instr);
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 0), ssbo_name, offset + " + 0");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 1), ssbo_name, offset + " + 1");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 2), ssbo_name, offset + " + 2");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 3), ssbo_name, offset + " + 3");
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX8: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX8: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = s_buffer_load_dword_offset(instr);
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 0), ssbo_name, offset + " + 0");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 1), ssbo_name, offset + " + 1");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 2), ssbo_name, offset + " + 2");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 3), ssbo_name, offset + " + 3");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 4), ssbo_name, offset + " + 4");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 5), ssbo_name, offset + " + 5");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 6), ssbo_name, offset + " + 6");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 7), ssbo_name, offset + " + 7");
            break;
        }

        case Shader::Opcode::S_BUFFER_LOAD_DWORDX16: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "S_BUFFER_LOAD_DWORDX16: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const auto offset = s_buffer_load_dword_offset(instr);
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 0), ssbo_name, offset + " +  0");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 1), ssbo_name, offset + " +  1");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 2), ssbo_name, offset + " +  2");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 3), ssbo_name, offset + " +  3");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 4), ssbo_name, offset + " +  4");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 5), ssbo_name, offset + " +  5");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 6), ssbo_name, offset + " +  6");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 7), ssbo_name, offset + " +  7");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 8), ssbo_name, offset + " +  8");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 9), ssbo_name, offset + " +  9");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 10), ssbo_name, offset + " + 10");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 11), ssbo_name, offset + " + 11");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 12), ssbo_name, offset + " + 12");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 13), ssbo_name, offset + " + 13");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 14), ssbo_name, offset + " + 14");
            code += std::format("{} = {}.data[{}];\n", getSGPR(instr.dst[0].code + 15), ssbo_name, offset + " + 15");
            break;
        }

        case Shader::Opcode::DS_WRITE_B32: {
            code += "// TODO: DS_WRITE_B32\n";
            break;
        }

        case Shader::Opcode::DS_WRITE2_B32: {
            code += "// TODO: DS_WRITE2_B32\n";
            break;
        }

        case Shader::Opcode::DS_WRITE2ST64_B32: {
            code += "// TODO: DS_WRITE2ST64_B32\n";
            break;
        }

        case Shader::Opcode::DS_SWIZZLE_B32: {
            code += setDST(instr.dst[0], std::format("{} /* TODO: DS_SWIZZLE_B32 */", getSRC(instr.src[0])));
            break;
        }

        case Shader::Opcode::DS_READ_B32: {
            code += "// TODO: DS_READ_B32\n";
            break;
        }

        case Shader::Opcode::DS_READ2_B32: {
            code += "// TODO: DS_READ2_B32\n";
            break;
        }

        case Shader::Opcode::DS_READ2ST64_B32: {
            code += "// TODO: DS_READ2ST64_B32\n";
            break;
        }

        case Shader::Opcode::DS_READ2_B64: {
            code += "// TODO: DS_READ2_B64\n";
            break;
        }

        case Shader::Opcode::BUFFER_LOAD_FORMAT_X:      // TODO: Handle properly
        case Shader::Opcode::BUFFER_LOAD_FORMAT_XY:     // TODO: Handle properly
        case Shader::Opcode::BUFFER_LOAD_FORMAT_XYZ:    // TODO: Handle properly
        case Shader::Opcode::BUFFER_LOAD_FORMAT_XYZW:   // TODO: Handle properly
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_X:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XY:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZ:
        case Shader::Opcode::TBUFFER_LOAD_FORMAT_XYZW: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "TBUFFER_LOAD_FORMAT_XYZW: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];
            addFetchBufferByteFunc(buf->binding);
            code += std::format("// DFMT: {} NFMT: {}\n", instr.control.mtbuf.dfmt, instr.control.mtbuf.nfmt);

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const std::string idx = instr.control.mtbuf.idxen ? getSRC<Type::Uint>(instr.src[0]) : "0";
            const std::string voffset = instr.control.mtbuf.offen ? getVGPR(instr.control.mtbuf.idxen ? instr.src[0].code + 1 : instr.src[0].code) : "0";
            const std::string instr_offs = std::format("{}", instr.control.mtbuf.offset);
            code += std::format("tmp_u = ({}) * getStrideForBinding({}) + {} + {};\n", idx, buf->binding, voffset, instr_offs);
            // TODO: soffset ???
            // TODO: Format conversion
            for (int elem = 0; elem < instr.control.mtbuf.count; elem++) {
                const std::string elem_addr = std::format("tmp_u + ({} << 2u)", elem);  // elem * sizeof(u32)
                code += std::format("{} = 0;\n", getVGPR(instr.src[1].code + elem));
                code += std::format("{} |= fetchBufferByte{}({} + 0) <<  0u;\n", getVGPR(instr.src[1].code + elem), buf->binding, elem_addr);
                code += std::format("{} |= fetchBufferByte{}({} + 1) <<  8u;\n", getVGPR(instr.src[1].code + elem), buf->binding, elem_addr);
                code += std::format("{} |= fetchBufferByte{}({} + 2) << 16u;\n", getVGPR(instr.src[1].code + elem), buf->binding, elem_addr);
                code += std::format("{} |= fetchBufferByte{}({} + 3) << 24u;\n", getVGPR(instr.src[1].code + elem), buf->binding, elem_addr);
            }
            break;
        }

        case Shader::Opcode::BUFFER_STORE_FORMAT_X:
        case Shader::Opcode::BUFFER_STORE_FORMAT_XY:
        case Shader::Opcode::BUFFER_STORE_FORMAT_XYZ:
        case Shader::Opcode::BUFFER_STORE_FORMAT_XYZW: {
            code += "// TODO: BUFFER_STORE_FORMAT\n";
            printf("TODO: BUFFER_STORE_FORMAT\n");
            break;
        }

        case Shader::Opcode::BUFFER_LOAD_DWORD:
        case Shader::Opcode::BUFFER_LOAD_DWORDX2:
        case Shader::Opcode::BUFFER_LOAD_DWORDX3:
        case Shader::Opcode::BUFFER_LOAD_DWORDX4: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "BUFFER_LOAD_DWORD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const std::string idx = instr.control.mubuf.idxen ? getSRC<Type::Uint>(instr.src[0]) : "0";
            const std::string voffset = instr.control.mubuf.offen ? getVGPR(instr.control.mubuf.idxen ? instr.src[0].code + 1 : instr.src[0].code) : "0";
            const std::string soffset = getSRC<Type::Uint>(instr.src[3]);
            const std::string instr_offs = std::format("{}", instr.control.mubuf.offset);
            code += std::format("tmp_u = ({}) * (getStrideForBinding({}) >> 2u) + {} + {} + ({} >> 2u);\n", idx, buf->binding, voffset, soffset, instr_offs);  // Is this right?

            for (int dword = 0; dword < instr.control.mubuf.count; dword++) {
                code += std::format("{} = {}.data[(tmp_u >> 0) + {}];\n", getVGPR(instr.src[1].code + dword), ssbo_name, dword);
            }
            break;
        }

        case Shader::Opcode::BUFFER_STORE_DWORD:
        case Shader::Opcode::BUFFER_STORE_DWORDX2:
        case Shader::Opcode::BUFFER_STORE_DWORDX3:
        case Shader::Opcode::BUFFER_STORE_DWORDX4: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "BUFFER_STORE_DWORD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto ssbo_name = std::format("ssbo{}", buf->binding);
            const std::string idx = instr.control.mubuf.idxen ? getSRC<Type::Uint>(instr.src[0]) : "0";
            const std::string voffset = instr.control.mubuf.offen ? getVGPR(instr.control.mubuf.idxen ? instr.src[0].code + 1 : instr.src[0].code) : "0";
            const std::string soffset = getSRC<Type::Uint>(instr.src[3]);
            const std::string instr_offs = std::format("{}", instr.control.mubuf.offset);
            code += std::format("tmp_u = ({}) * (getStrideForBinding({}) >> 2u) + {} + {} + ({} >> 2u);\n", idx, buf->binding, voffset, soffset, instr_offs);  // Is this right?

            for (int dword = 0; dword < instr.control.mubuf.count; dword++) {
                code += std::format("{}.data[(tmp_u >> 0) + {}] = {};\n", ssbo_name, dword, getVGPR(instr.src[1].code + dword));
            }
            break;
        }

        case Shader::Opcode::IMAGE_STORE: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_STORE: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto image_name = std::format("tex{}", buf->binding);

            // TODO: Don't duplicate this stuff from IMAGE_SAMPLE (is it even the same???)
            int coord_reg_idx = instr.src[0].code;

            // TODO: Does IMAGE_STORE use DMASK?
            code += std::format("tmp.x = u2f({});\n", getVGPR(instr.dst[0].code + 0));
            code += std::format("tmp.y = u2f({});\n", getVGPR(instr.dst[0].code + 1));
            code += std::format("tmp.z = u2f({});\n", getVGPR(instr.dst[0].code + 2));
            code += std::format("tmp.w = u2f({});\n", getVGPR(instr.dst[0].code + 3));

            const std::string texcoords = std::format("ivec2({}, {})", getVGPR(coord_reg_idx), getVGPR(coord_reg_idx + 1));
            code += std::format("// T# is in s{}\n", instr.src[2].code * 4);
            code += std::format("imageStore({}, {}, tmp);\n", image_name, texcoords);
            break;
        }

        case Shader::Opcode::IMAGE_ATOMIC_ADD: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_ATOMIC_ADD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto image_name = std::format("tex{}", buf->binding);

            code += "// TODO: IMAGE_ATOMIC_ADD " + image_name + "\n";
            break;
        }

        case Shader::Opcode::IMAGE_ATOMIC_UMIN: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_ATOMIC_UMIN: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto image_name = std::format("tex{}", buf->binding);

            code += "// TODO: IMAGE_ATOMIC_UMIN " + image_name + "\n";
            break;
        }

        case Shader::Opcode::IMAGE_SAMPLE_L:
        case Shader::Opcode::IMAGE_SAMPLE_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C:
        case Shader::Opcode::IMAGE_SAMPLE_O:
        case Shader::Opcode::IMAGE_SAMPLE_LZ_O:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ:
        case Shader::Opcode::IMAGE_SAMPLE_C_LZ_O:
            code += "// IMAGE_SAMPLE_*\n";
        case Shader::Opcode::IMAGE_SAMPLE: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_SAMPLE: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto sampler_name = std::format("tex{}", buf->binding);

            // TODO: We use whatever T# the game set the first time the shader is compiled. In theory it can change.
            auto* tsharp = buf->desc_info.asPtr<TSharp>();
            const bool is_3d = tsharp ? tsharp->type == 10 : false;

            const auto flags = MimgModifierFlags(instr.control.mimg.mod);
            const bool offset = flags.test(MimgModifier::Offset);
            const bool lod_bias = flags.test(MimgModifier::LodBias);
            const bool pcf = flags.test(MimgModifier::Pcf);
            const bool derivative = flags.test(MimgModifier::Derivative);

            int coord_reg_idx = instr.src[0].code;

            int offset_reg = 0;
            std::string offset_str = "";
            if (offset) {
                offset_reg = coord_reg_idx++;
                offset_str = std::format("unpackImageOffset({})", getVGPR(offset_reg));
            }

            if (lod_bias)
                coord_reg_idx++;

            if (pcf)
                coord_reg_idx++;

            if (derivative)
                coord_reg_idx += !is_3d ? 2 : 3; // TODO: This should be 1 per dimension (i.e. 3 for 3D textures)

            std::string sample_op = "texture";

            // TODO: LoadBias, PCF, derivative
            // TODO: Other dimensions
            std::string texcoords;
            if (!is_3d)
                texcoords = std::format("vec2(u2f({}), u2f({}))", getVGPR(coord_reg_idx), getVGPR(coord_reg_idx + 1));
            else
                texcoords = std::format("vec3(u2f({}), u2f({}), u2f({}))", getVGPR(coord_reg_idx), getVGPR(coord_reg_idx + 1), getVGPR(coord_reg_idx + 2));

            code += std::format("// T# is in s{}\n", instr.src[2].code * 4);

            // Instead of using textureOffset we bake the offset into the coordinates, because textureOffset expects a constant offset
            if (offset)
                // Resident Evil Revelations relies on the + 0.5.
                // It samples at (0, 0) with offset (1, 0) expecting to receive the texel at (1, 0). This would work if we used textureOffset, because the offset is applied in texel-space.
                // Because we add the offset to the normalized coordinates instead, it doesn't work, because (0, 0) is the top-left corner of the texture 
                // rather than the center of the texel, so adding (1, 0) to it ends up falling in between the 2 pixels and doesn't reliably sample the correct pixel.
                texcoords = std::format("{} + (vec2({}.xy) + 0.5) * (1.0 / vec2(textureSize({}, 0)))", texcoords, offset_str, sampler_name);    // TODO: Multiple dimensions

            std::string operation = std::format("{}({}, {}", sample_op, sampler_name, texcoords);
            // TODO: Add other parameters for other sampling options
            operation += ")";

            code += std::format("tmp = {};\n", operation);
            code += "tmp2 = float[](tmp.x, tmp.y, tmp.z, tmp.w);\n";

            // Set results according to DMASK
            u32 dest_gpr_offs = 0;
            for (int channel = 0; channel < 4; channel++) {
                if (((instr.control.mimg.dmask >> channel) & 1) == 0)
                    continue;

                code += std::format("{} = f2u(tmp2[{}]);\n", getVGPR(instr.dst[0].code + dest_gpr_offs++), channel);
            }
            break;
        }

        case Shader::Opcode::IMAGE_LOAD:
        case Shader::Opcode::IMAGE_LOAD_MIP: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_LOAD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            const auto sampler_name = std::format("tex{}", buf->binding);
            const std::string texcoords = std::format("ivec2({}, {})", getVGPR(instr.src[0].code), getVGPR(instr.src[0].code + 1));
            code += std::format("tmp = texelFetch({}, {}, 0);\n", sampler_name, texcoords);
            code += "tmp2 = float[](tmp.x, tmp.y, tmp.z, tmp.w);\n";

            // Set results according to DMASK
            u32 dest_gpr_offs = 0;
            for (int channel = 0; channel < 4; channel++) {
                if (((instr.control.mimg.dmask >> channel) & 1) == 0)
                    continue;

                code += std::format("{} = f2u(tmp2[{}]);\n", getVGPR(instr.dst[0].code + dest_gpr_offs++), channel);
            }
            break;
        }

        case Shader::Opcode::IMAGE_GET_RESINFO: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_LOAD: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];
            auto* tsharp = buf->desc_info.asPtr<TSharp>();
            const bool is_3d = tsharp ? tsharp->type == 10 : false;

            const auto sampler_name = std::format("tex{}", buf->binding);

            if (!is_3d)
                code += std::format("itmp.xy = textureSize({}, 0);\n", sampler_name);
            else
                code += std::format("itmp.xyz = textureSize({}, 0);\n", sampler_name);

            u32 dest_gpr_offs = 0;
            if ((instr.control.mimg.dmask >> (u32)ImageResComponent::Width) & 1) {
                code += std::format("{} = itmp.x;\n", getVGPR(instr.dst[0].code + dest_gpr_offs++));
            }

            if ((instr.control.mimg.dmask >> (u32)ImageResComponent::Height) & 1) {
                code += std::format("{} = itmp.y;\n", getVGPR(instr.dst[0].code + dest_gpr_offs++));
            }

            if ((instr.control.mimg.dmask >> (u32)ImageResComponent::Depth) & 1) {
                if (!is_3d) Helpers::panic("IMAGE_GET_RESINFO: requested depth for 2d texture\n");
                code += std::format("{} = itmp.z;\n", getVGPR(instr.dst[0].code + dest_gpr_offs++));
            }

            if ((instr.control.mimg.dmask >> (u32)ImageResComponent::MipCount) & 1) {
                code += std::format("{} = 1; // TODO: IMAGE_GET_RESINFO mip_count\n", getVGPR(instr.dst[0].code + dest_gpr_offs++));
            }
            break;
        }

        case Shader::Opcode::IMAGE_GATHER4_LZ:
        case Shader::Opcode::IMAGE_GATHER4_C: {
            const auto buffer_mapping = pc;
            Helpers::debugAssert(buffer_map.contains(buffer_mapping), "IMAGE_GATHER4: no buffer_mapping");  // Unreachable if everything works as intended
            auto* buf = buffer_map[buffer_mapping];

            code += std::format("// T# is in s{}\n", instr.src[2].code * 4);

            const auto sampler_name = std::format("tex{}", buf->binding);
            const std::string texcoords = std::format("vec2(u2f({}), u2f({}))", getVGPR(instr.src[0].code), getVGPR(instr.src[0].code + 1));

            // For gather instructions dmask selects the component
            int comp = 0;
            if (std::popcount(instr.control.mimg.dmask) == 1) {
                for (comp = 0; comp < 4; comp++) {
                    if (instr.control.mimg.dmask >> comp)
                        break;
                }
            }
            else {
                Helpers::panic("IMAGE_GATHER4: dmask has != 1 bits set");
            }

            code += std::format("tmp = textureGather({}, {}, {});\n", sampler_name, texcoords, comp);
            code += "tmp2 = float[](tmp.x, tmp.y, tmp.z, tmp.w);\n";

            // Set results (ignore DMASK because it's used for the component here)
            u32 dest_gpr_offs = 0;
            for (int channel = 0; channel < 4; channel++) {
                code += std::format("{} = f2u(tmp2[{}]);\n", getVGPR(instr.dst[0].code + dest_gpr_offs++), channel);
            }
            break;
        }

        case Shader::Opcode::IMAGE_GET_LOD: {
            std::format("{} = 0; // TODO: IMAGE_GET_LOD\n", getVGPR(instr.dst[0].code + 0));
            std::format("{} = 0; // TODO: IMAGE_GET_LOD\n", getVGPR(instr.dst[0].code + 1));
            printf("TODO: IMAGE_GET_LOD\n");
            break;
        }

        case Shader::Opcode::EXP: {
            if (stage == ShaderStage::Fragment) {
                if (instr.control.exp.vm) {
                    code += "if (exec == 0) discard;\n";
                }
            }

            const auto tgt = instr.control.exp.target;

            // When compr is enabled, EXP uses four 16bit values packed in VSRC0 and VSRC1 instead of four 32bit values in VSRC0, VSRC1, VSRC2 and VSRC3
            std::string data;
            if (!instr.control.exp.compr)
                data = std::format("vec4({}, {}, {}, {})", getSRC(instr.src[0]), getSRC(instr.src[1]), getSRC(instr.src[2]), getSRC(instr.src[3]));
            else
                data = std::format("vec4(unpackHalf2x16({}), unpackHalf2x16({}))", getSRC<Type::Uint>(instr.src[0]), getSRC<Type::Uint>(instr.src[1]));

            // Color targets
            if (tgt >= 0 && tgt < 8) {
                const std::string attr = std::format("col{}", tgt);
                addOutAttr(attr, "vec4", tgt);
                code += std::format("{} = {};\n", attr, data);
            }
            // Output to Z
            else if (tgt == 8) {
                code += "gl_FragDepth = " + data + ".x;\n";
            }
            else if (tgt == 9) {
                // "Output to NULL" - do nothing
            }
            // Output pos0
            else if (tgt == 12) {
                code += "gl_Position = " + data + ";\n";
            }
            // Output pos1-pos4, ignore for now
            else if (tgt >= 13 && tgt <= 15) {
                code += "// TODO: pos1-pos4 output\n";
            }
            // Output attribute
            else if (tgt >= 32 && tgt < 64) {
                const std::string attr = std::format("ps_attr{}", tgt - 32);
                addOutAttr(attr, "vec4", tgt - 32);
                code += std::format("{} = {};\n", attr, data);
            }
            else Helpers::panic("ShaderDecompiler: EXP unhandled tgt %d\n", tgt);
            break;
        }

        default: {
            //printf("BasicBlock so far:\n%s\n", code.c_str());
            //Helpers::panic("Unimplemented shader instruction %d\n", instr.opcode);
            code += "// TODO\n";
        }
        }

        // Increment pc
        pc += instr.length;
    }
}

std::string branchCondition(GcnInst& instr) {
    switch (instr.opcode) {
    case Shader::Opcode::S_CBRANCH_EXECZ: return "exec == 0";
    case Shader::Opcode::S_CBRANCH_VCCNZ: return "vcc != 0";
    case Shader::Opcode::S_CBRANCH_SCC0:  return "scc == 0";
    case Shader::Opcode::S_CBRANCH_SCC1:  return "scc == 1";
    case Shader::Opcode::S_CBRANCH_VCCZ:  return "vcc == 0";
    
    default: {
        //Helpers::panic("Unhandled branch condition for instruction %d\n", instr.opcode);
        return "true";
    }
    }
}

// Keep track of emitted blocks
std::unordered_set<BasicBlock*> emitted_blocks;
std::string emit(BasicBlock* from, BasicBlock* to) {
    std::string code;
    if (from != to) {
        if (emitted_blocks.contains(from))
            return "";

        const bool is_conditional = from->branch_to && from->fallthrough;
        code += from->code;
        emitted_blocks.insert(from);

        if (is_conditional) {
            // The merge block is the immediate post-dominator of the block
            auto* merge = from->immediate_post_dominator;

            // Decompile condition
            const auto cond = branchCondition(from->branch_instr);

            // Check if either branch_to or fallthrough is the merge block. In this case, we emit an if.
            if (from->branch_to == merge) {
                // We need to negate the condition (the conditional code is in the fallthrough block, so when the branch condition is false)
                const auto negated_cond = "!(" + cond + ")";

                code += std::format("if ({}) {{\n", negated_cond);
                code += emit(from->fallthrough, merge);     // Emit until the merge block
                code += "}\n";
            }
            else if (from->fallthrough == merge) {
                // No need to negate the condition
                code += std::format("if ({}) {{\n", cond);
                code += emit(from->branch_to, merge);     // Emit until the merge block
                code += "}\n";
            }
            else {
                code += std::format("if ({}) {{\n", cond);
                code += emit(from->branch_to, merge);     // Emit until the merge block
                code += "} else {\n";
                code += emit(from->fallthrough, merge);
                code += "}\n";
            }

            // Continue emitting from the merge block
            code += emit(merge, to);
        }
        else {
            // Unconditional. Emit the current block and it's successor, which is either the fallthrough block or the branch_to block in case of unconditional branches.
            auto* successor = from->branch_to ? from->branch_to : from->fallthrough;
            if (successor) {
                code += emit(successor, to);
            }
            else {
                if (!from->is_end)
                    printf("WARNING: basic block has no successor but it is not the end block\n");
            }
        }
    }
    return code;
}

void decompileShader(u32* data, ShaderStage stage, ShaderData& out_data, FetchShader* fetch_shader, ComputeJob* compute_job) {
    //std::ofstream out;
    //if (stage == ShaderStage::Vertex) {
    //  out.open("shader.bin", std::ios::binary);
    //  out.write((char*)data, 8_KB);
    //  out.close();
    //}
    Shader::GcnDecodeContext decoder;
    Shader::GcnCodeSlice code_slice = Shader::GcnCodeSlice((u32*)data, data + std::numeric_limits<u32>::max());

    shader.clear();
    shader.reserve(256_KB);  // Avoid reallocations
    blocks.clear();
    block_map.clear();
    block_entries.clear();
    emitted_blocks.clear();
    const_tables.clear();
    const_tables.reserve(1_KB);
    initialization.clear();
    const_table_map.clear();
    in_attrs.clear();
    out_attrs.clear();
    in_buffers.clear();
    in_samplers.clear();
    out_imgs.clear();
    has_cubemap_id_func = false;
    has_cubemap_scoord_func = false;
    has_cubemap_tcoord_func = false;
    has_cubemap_majoraxis_func = false;
    has_fetch_buffer_func_map.clear();
    vgpr_map.clear();
    sgpr_map.clear();
    lane_map.clear();

    shader += R"(
#version 450
#extension GL_ARB_shading_language_packing : require

#define u2f(x) uintBitsToFloat(x)
#define f2u(x) floatBitsToUint(x)

#define PI 3.14159265358979323846

vec4 tmp;
ivec4 itmp;
float tmp2[4];
uint tmp_u;

uint scc;
uint vcc;
uint exec;

layout(push_constant, std430) uniform BufferInfo {
    uint stride[24];
    uint fmt[32];
} buf_info;

uint getStrideForBinding(uint binding) {
    uint stride = buf_info.stride[binding >> 1u];
    if (binding % 2 == 1)
        stride >>= 16u;
    else
        stride &= 0xffffu;
    return stride;
}

int sext6(uint x) {
    return int(x << 26) >> 26;
}

ivec3 unpackImageOffset(uint packed) {
    return ivec3(
        sext6((packed >> 0)  & 0x3fu),
        sext6((packed >> 8)  & 0x3fu),
        sext6((packed >> 16) & 0x3fu)
    );
}

)";

    if (stage == ShaderStage::Compute) {
        shader += std::format("layout(local_size_x = {}, local_size_y = {}, local_size_z = {}) in;\n\n", compute_job->n_threads_x, compute_job->n_threads_y, compute_job->n_threads_z);
    }

    trackAndCreateBuffers(stage, out_data, decoder, code_slice);

    std::string main;
    main.reserve(32_KB); // Avoid reallocations
    
    main += "vcc  = 0;\n";
    main += "scc  = 0;\n";
    main += "exec = 1;\n";
    if (stage == ShaderStage::Vertex) {
        getVGPR(0);
        main += "v0 = gl_VertexIndex;\n";
    } else if (stage == ShaderStage::Fragment) {
        getVGPR(2);
        getVGPR(3);
        main += "v2 = f2u(gl_FragCoord.x);\n";
        main += "v3 = f2u(gl_FragCoord.y);\n";
    }
    else if (stage == ShaderStage::Compute) {
        getSGPR(16);    // TODO: Register numbers hardcoded from Minecraft
        getSGPR(17);
        getSGPR(18);
        getVGPR(0);
        getVGPR(1);
        getVGPR(2);
        main += "s16 = gl_WorkGroupID.x;\n";
        main += "v0 = gl_LocalInvocationID.x;\n";   // TODO: Or is it the opposite? (local invocation id in SGPRs)
        main += "s17 = gl_WorkGroupID.y;\n";
        main += "v1 = gl_LocalInvocationID.y;\n";
        main += "s18 = gl_WorkGroupID.z;\n";
        main += "v2 = gl_LocalInvocationID.z;\n";
    }

    switch (stage) {
    case ShaderStage::Vertex: {
        // Write vertex inputs from fetch shader
        for (auto& binding : fetch_shader->bindings) {
            VSharp* vsharp = binding.vsharp_loc.asPtr();
            std::string attr = std::format("vs_attr{}", binding.idx);
            auto type = getType(binding.n_elements, vsharp->nfmt);
            addInAttr(attr, type, binding.idx);

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

                if (!(type.contains("ivec") || type.contains("uvec") || type.contains("int")))
                    val = std::format("floatBitsToUint({})", val);

                main += std::format("{} = {};\n", getVGPR(binding.dest_vgpr + i), val);
            }
        }
        break;
    }
    }

    main += "\n";

    // Discover basic block entry points
    discoverBasicBlockEntries(data, 0);

    BasicBlock* start = blocks.emplace_back(std::make_unique<BasicBlock>()).get();
    decompileBasicBlock(data, 0, stage, *start);
    
    // Populate predecessors
    for (auto& block : blocks) {
        if (block->branch_to)
            block->branch_to->predecessors.push_back(block.get());
    
        if (block->fallthrough)
            block->fallthrough->predecessors.push_back(block.get());
    }

    // Initialize post-dominators
    // Our algorithm expects every block's post-dominator set to contain every block at first, other than the exit which only post-dominates itself.
    // It will then shrink them (see below).
    std::unordered_set<BasicBlock*> all_blocks;
    for (auto& block : blocks)
        all_blocks.insert(block.get());
    for (auto& block : blocks) {
        if (!block->is_end)
            block->post_dominators = all_blocks;
        else
            block->post_dominators.insert(block.get());     // The end block only post-dominates itself
    }

    // Compute post-dominator tree
    // For every block, its post-dominator set is made up of the block itself + the intersection of the post-dominators of its successors.
    // Repeat this until there are no changes, meaning we computed the entire graph.
    bool changed;
    do {
        changed = false;

        for (auto& block : blocks) {
            if (block->is_end) continue;

            // Build successor list
            std::vector<BasicBlock*> successors;
            if (block->branch_to)
                successors.push_back(block->branch_to);
            if (block->fallthrough)
                successors.push_back(block->fallthrough);

            // In theory, the only block that has 0 successors is the end (which we skip),
            // so it should be safe to assume successors contains at least 1 block.
            std::unordered_set<BasicBlock*> new_set = successors[0]->post_dominators;

            // Intersect successors
            for (int i = 1; i < successors.size(); i++) {
                std::unordered_set<BasicBlock*> tmp;
                for (auto& postdom : new_set) {
                    if (successors[i]->post_dominators.contains(postdom)) {
                        tmp.insert(postdom);
                    }
                }
                new_set = tmp;
            }

            // Insert self
            new_set.insert(block.get());

            // Compare new set with old set
            if (new_set != block->post_dominators) {
                changed = true;
                block->post_dominators = new_set;
            }
        }
    } while (changed);

    // Compute immediate post-dominators
    // For every block's post-dominator, the post-dominator is NOT the immediate post-dominator if another candidate's post-dominator set contains it.
    for (auto& block : blocks) {
        for (auto& postdom : block->post_dominators) {
            if (postdom == block.get()) continue;

            bool is_immediate = true;
            for (auto& other_postdom : block->post_dominators) {
                if (other_postdom == block.get())   continue;
                if (other_postdom == postdom)       continue;

                if (other_postdom->post_dominators.contains(postdom)) {
                    is_immediate = false;
                    break;
                }
            }

            if (is_immediate) {
                if (block->immediate_post_dominator != nullptr) {
                    Helpers::panic("Shader::decompileShader: detected non-structured control flow (basic block has multiple immediate post-dominators)\n");
                }

                block->immediate_post_dominator = postdom;
            }
        }
    }
    
    main += emit(start, nullptr);

    shader += "\n";
    shader += const_tables;
    shader += "\n";

    // Print immediate post-dominators
    for (auto& block : blocks) {
        shader += std::format("// immediate post dominator of {}: {}\n", block->pc, block->immediate_post_dominator ? std::format("{}", block->immediate_post_dominator->pc) : "nullptr");
    }
    shader += "\n";

    main = initialization + "\n" + main;

    // Declare main function
    addFunc("void main", main);

    //printf("%s\n", shader.c_str());
    out_data.source = shader;

    if (stage == ShaderStage::Vertex) {
        // Save vertex outputs
        out_data.vtx_outputs.reserve(32);
        for (auto& [location, name] : out_attrs)
            out_data.vtx_outputs.push_back(location);
    }
}

}   // End namespace PS4::GCN::Shader
