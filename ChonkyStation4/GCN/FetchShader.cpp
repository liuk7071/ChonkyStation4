#include "FetchShader.hpp"
#include <Logger.hpp>
#include <GCN/GCN.hpp>
#include <GCN/Shader/Decoder.hpp>
#include <GCN/VSharp.hpp>
#include <unordered_map>


namespace PS4::GCN {

MAKE_LOG_FUNCTION(log, gcn_fetch_shader);

FetchShader::FetchShader(const u8* data) {
    Shader::GcnDecodeContext decoder;
    Shader::GcnCodeSlice code_slice((u32*)data, (u32*)data + std::numeric_limits<u32>::max());

    // The instructions we care about are the LOAD_DWORD and BUFFER_LOAD_FORMAT_* instructions.
    // LOAD_DWORD instructions tell us the address of the V# in memory. V# are vertex attribute descriptors.
    // Example: S_LOAD_DWORDX4 s[4:7], s[2:3], 0x04
    // s[2:3] is the base address, while 0x04 is an offset in DWORDs. So the final address is, in this case, s[2:3] + 4 * sizeof(u64)
    //
    // BUFFER_LOAD_FORMAT_* instructions tell us what VGPRs to bind the attribute to, as well as additional offsets inside the buffer.
    // Example: BUFFER_LOAD_FORMAT_XYZW v[4:7], v0, s[4:7], 0, [0] IDXEN
    // v[4:7] = dest regs
    // v0 = vindex
    // s[4:7] = buffer descriptor (V#)
    // 0 = soffset
    // [0] = inst_offset
    // idxen = enable vindex
    // The final address of the vetex fetch is calculated as the following:
    // vsharp.base + soffset + vindex * stride + voffset + inst_offset

    // Here we map SGPRs to V# pointers
    std::unordered_map<u32, VSharpLocation> vsharps;

    while (!code_slice.atEnd()) {
        const auto instr = decoder.decodeInstruction(code_slice);

        if (instr.inst_class == Shader::InstClass::ScalarMemRd) {
            const auto sgpr_pair = instr.src[0].code * 2;
            //VSharp* vsharp_ptr;
            //std::memcpy(&vsharp_ptr, &renderer->regs[Reg::mmSPI_SHADER_USER_DATA_VS_0], sizeof(VSharp*));
            //vsharp_ptr = (VSharp*)((u64*)vsharp_ptr + instr.control.smrd.offset);  // The immediate is an offset in dwords
            vsharps[sgpr_pair] = { sgpr_pair, instr.control.smrd.offset };
            log("Loaded V# in SGPR pair %d\n", sgpr_pair);
            continue;
        }

        if (instr.inst_class == Shader::InstClass::VectorMemBufFmt) {
            auto& binding = bindings.emplace_back();
            binding.vsharp_loc = vsharps[instr.src[2].code * 4];
            binding.dest_vgpr = instr.src[1].code;
            binding.n_elements = instr.control.mubuf.count;
            binding.soffs = instr.src[3].code;
            binding.voffs = instr.src[0].code;  // TODO: Verify this, but I'm pretty sure
            log("Binding for VGPR %d: %d elements\n", binding.dest_vgpr, binding.n_elements);
            continue;
        }

        if (instr.opcode == Shader::Opcode::S_SETPC_B64) {
            break;
        }
    }
}

}   // End namespace PS4::GCN