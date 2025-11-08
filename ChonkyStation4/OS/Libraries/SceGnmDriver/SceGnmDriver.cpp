#include "SceGnmDriver.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <GCN/PM4.hpp>


namespace PS4::OS::Libs::SceGnmDriver {

MAKE_LOG_FUNCTION(log, lib_sceGnmDriver);

void init(Module& module) {
    module.addSymbolExport("yb2cRhagD1I", "sceGnmDrawInitDefaultHardwareState350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawInitDefaultHardwareState350);
    module.addSymbolExport("+AFvOEXrKJk", "sceGnmSetEmbeddedVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetEmbeddedVsShader);
    module.addSymbolExport("X9Omw9dwv5M", "sceGnmSetEmbeddedPsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetEmbeddedPsShader);
    module.addSymbolExport("GGsn7jMTxw4", "sceGnmDrawIndexAuto", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndexAuto);
    module.addSymbolExport("oYM+YzfCm2Y", "sceGnmDrawIndexOffset", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndexOffset);
    module.addSymbolExport("gAhCn6UiU4Y", "sceGnmSetVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetVsShader);
    module.addSymbolExport("5uFKckiJYRM", "sceGnmSetPsShader350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetPsShader350);
}

s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState350(u32* buf, u32 size) {
    log("sceGnmDrawInitDefaultHardwareState350(buf=%p, size=0x%x) TODO\n", buf, size);

    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetEmbeddedVsShader(u32* buf, u32 size, u32 shader_id, u32 shader_modifier) {
    log("sceGnmSetEmbeddedVsShader(buf=%p, size=0x%x, shader_id=%d, shader_modifier=0x%x) TODO\n", buf, size, shader_id, shader_modifier);

    // Write a long NOP
    *buf = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetEmbeddedPsShader(u32* buf, u32 size, u32 shader_id, u32 shader_modifier) {
    log("sceGnmSetEmbeddedPsShader(buf=%p, size=0x%x, shader_id=%d, shader_modifier=0x%x) TODO\n", buf, size, shader_id, shader_modifier);

    // Write a long NOP
    *buf = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size);
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

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x48;
    *buf++ = vs_regs[0];
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

    *buf = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 11); // Trailing NOPs
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetPsShader350(u32* buf, u32 size, const u32* ps_regs) {
    log("sceGnmSetPsShader350(buf=%p, size=0x%x, ps_regs=%p)\n", buf, size, ps_regs);

    if (!ps_regs) {
        Helpers::panic("sceGnmSetPsShader350: ps_regs is nullptr (TODO)\n");
    }

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::SetShReg, 4);
    *buf++ = 0x8;
    *buf++ = ps_regs[0];
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
    
    *buf = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 11); // Trailing NOPs
    return SCE_OK;
}

} // End namespace SceGnmDriver