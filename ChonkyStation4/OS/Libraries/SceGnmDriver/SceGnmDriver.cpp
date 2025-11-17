#include "SceGnmDriver.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <GCN/PM4.hpp>
#include <GCN/CommandProcessor.hpp>


namespace PS4::OS::Libs::SceGnmDriver {

MAKE_LOG_FUNCTION(log, lib_sceGnmDriver);

void init(Module& module) {
    module.addSymbolExport("xbxNatawohc", "sceGnmSubmitAndFlipCommandBuffers", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSubmitAndFlipCommandBuffers);
    module.addSymbolExport("yvZ73uQUqrk", "sceGnmSubmitDone", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSubmitDone);
    module.addSymbolExport("yb2cRhagD1I", "sceGnmDrawInitDefaultHardwareState350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawInitDefaultHardwareState350);
    module.addSymbolExport("+AFvOEXrKJk", "sceGnmSetEmbeddedVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetEmbeddedVsShader);
    module.addSymbolExport("X9Omw9dwv5M", "sceGnmSetEmbeddedPsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetEmbeddedPsShader);
    module.addSymbolExport("GGsn7jMTxw4", "sceGnmDrawIndexAuto", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndexAuto);
    module.addSymbolExport("oYM+YzfCm2Y", "sceGnmDrawIndexOffset", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndexOffset);
    module.addSymbolExport("gAhCn6UiU4Y", "sceGnmSetVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetVsShader);
    module.addSymbolExport("5uFKckiJYRM", "sceGnmSetPsShader350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetPsShader350);
}

s32 PS4_FUNC sceGnmSubmitAndFlipCommandBuffers(u32 cnt, u32** dcb_gpu_addrs, u32* dcb_sizes, u32** ccb_gpu_addrs, u32* ccb_sizes, u32 video_out_handle, u32 buf_idx, u32 flip_mode, u64 flip_arg) {
    log("sceGnmSubmitAndFlipCommandBuffers(cnt=%d, dcb_gpu_addrs=*%p, dcb_sizes=%p, ccb_gpu_addrs=*%p, ccb_sizes=%p, video_out_handle=%d, buf_idx=%d, flip_mode=%d, flip_arg=0x%llx)\n", cnt, dcb_gpu_addrs, dcb_sizes, ccb_gpu_addrs, ccb_sizes, video_out_handle, buf_idx, flip_mode, flip_arg);
    
    for (int i = 0; i < cnt; i++)
        GCN::processCommands(dcb_gpu_addrs[i], dcb_sizes[i], ccb_gpu_addrs ? ccb_gpu_addrs[i] : nullptr, ccb_sizes ? ccb_sizes[i] : 0);

    GCN::renderer->flip();
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSubmitDone() {
    log("sceGnmSubmitDone()\n");
    
    return SCE_OK;
}

s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState350(u32* buf, u32 size) {
    log("sceGnmDrawInitDefaultHardwareState350(buf=%p, size=0x%x) TODO\n", buf, size);

    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetEmbeddedVsShader(u32* buf, u32 size, u32 shader_id, u32 shader_modifier) {
    log("sceGnmSetEmbeddedVsShader(buf=%p, size=0x%x, shader_id=%d, shader_modifier=0x%x) TODO\n", buf, size, shader_id, shader_modifier);

    // TODO: This is stubbed for now. I just set the shader pointers to null
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x48;
    *buf++ = 0;
    *buf++ = 0;
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x49;
    *buf++ = 0;
    *buf++ = 0;
    // Write a long NOP
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size - 8);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetEmbeddedPsShader(u32* buf, u32 size, u32 shader_id, u32 shader_modifier) {
    log("sceGnmSetEmbeddedPsShader(buf=%p, size=0x%x, shader_id=%d, shader_modifier=0x%x) TODO\n", buf, size, shader_id, shader_modifier);

    // TODO: This is stubbed for now. I just set the shader pointers to null
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x48;
    *buf++ = 0;
    *buf++ = 0;
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x49;
    *buf++ = 0;
    *buf++ = 0;
    // Write a long NOP
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size - 8);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmDrawIndexAuto(u32* buf, u32 size, u32 cnt, u32 flags) {
    log("sceGnmDrawIndexAuto(buf=%p, size=0x%x, cnt=%d, flags=0x%x)\n", buf, size, cnt, flags);

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::DrawIndexAuto, size);
    *buf++ = cnt;
    return SCE_OK;
}

s32 PS4_FUNC sceGnmDrawIndexOffset(u32* buf, u32 size, u32 start, u32 cnt, u32 flags) {
    log("sceGnmDrawIndexOffset(buf=%p, size=0x%x, start=%d, cnt=%d, flags=0x%x)\n", buf, size, start, cnt, flags);

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::DrawIndexOffset2, size);
    *buf++ = cnt;
    *buf++ = start;
    *buf++ = cnt;
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetVsShader(u32* buf, u32 size, const u32* vs_regs, u32 shader_modifier) {
    log("sceGnmSetVsShader(buf=%p, size=0x%x, vs_regs=%p, shader_modifier=0x%x)\n", buf, size, vs_regs, shader_modifier);
    log("addr_lo=0x%08x\n", vs_regs[0]);
    log("addr_hi=0x%08x\n", vs_regs[1]);

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x48;
    *buf++ = vs_regs[0];
    *buf++ = 0;
    // TODO: libSceGnmDriver.sprx does NOT write to the hi addr register, instead it asserts if it's non-zero.
    // We can't do that because we don't implement the virtual memory map properly yet, so our addresses aren't going to fit
    // in the lo register alone.
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x49;
    *buf++ = vs_regs[1];
    *buf++ = 0;
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x4a;
    *buf++ = shader_modifier == 0 ? vs_regs[2] : (vs_regs[2] & 0xfcfffc3f) | shader_modifier;
    *buf++ = vs_regs[3];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x207;
    *buf++ = vs_regs[6];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x1b1;
    *buf++ = vs_regs[4];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x1c3;
    *buf++ = vs_regs[5];

    // TODO: REMOVE THE -4 WHEN I FIX THE TODO ABOVE!!!!!
    *buf = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 12 - 4); // Trailing NOPs
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetPsShader350(u32* buf, u32 size, const u32* ps_regs) {
    log("sceGnmSetPsShader350(buf=%p, size=0x%x, ps_regs=%p)\n", buf, size, ps_regs);
    log("addr_lo=0x%08x\n", ps_regs[0]);
    log("addr_hi=0x%08x\n", ps_regs[1]);

    if (!ps_regs) {
        Helpers::panic("sceGnmSetPsShader350: ps_regs is nullptr (TODO)\n");
    }

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x8;
    *buf++ = ps_regs[0];
    *buf++ = 0;
    // TODO: libSceGnmDriver.sprx does NOT write to the hi addr register, instead it asserts if it's non-zero.
    // We can't do that because we don't implement the virtual memory map properly yet, so our addresses aren't going to fit
    // in the lo register alone.
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x9;
    *buf++ = ps_regs[1];
    *buf++ = 0;
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0xa;
    *buf++ = ps_regs[2];
    *buf++ = ps_regs[3];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 4);
    *buf++ = 0x1c4;
    *buf++ = ps_regs[4];
    *buf++ = ps_regs[5];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 4);
    *buf++ = 0x1b3;
    *buf++ = ps_regs[6];
    *buf++ = ps_regs[7];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x1b6;
    *buf++ = ps_regs[8];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x1b8;
    *buf++ = ps_regs[9];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x203;
    *buf++ = ps_regs[10];
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetContextReg, 3);
    *buf++ = 0x8f;
    *buf++ = ps_regs[11];
    
    // TODO: REMOVE THE -4 WHEN I FIX THE TODO ABOVE!!!!!
    *buf = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 12 - 4); // Trailing NOPs
    return SCE_OK;
}

} // End namespace SceGnmDriver