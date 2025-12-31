#include "SceGnmDriver.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <GCN/GCN.hpp>
#include <GCN/PM4.hpp>


namespace PS4::OS::Libs::SceGnmDriver {

MAKE_LOG_FUNCTION(log, lib_sceGnmDriver);

void init(Module& module) {
    module.addSymbolExport("xbxNatawohc", "sceGnmSubmitAndFlipCommandBuffers", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSubmitAndFlipCommandBuffers);
    module.addSymbolExport("yvZ73uQUqrk", "sceGnmSubmitDone", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSubmitDone);
    module.addSymbolExport("b0xyllnVY-I", "sceGnmAddEqEvent", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmAddEqEvent);
    module.addSymbolExport("29oKvKXzEZo", "sceGnmMapComputeQueue", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmMapComputeQueue);
    module.addSymbolExport("bX5IbRvECXk", "sceGnmDingDong", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDingDong);
    module.addSymbolExport("Idffwf3yh8s", "sceGnmDrawInitDefaultHardwareState", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawInitDefaultHardwareState);
    module.addSymbolExport("0H2vBYbTLHI", "sceGnmDrawInitDefaultHardwareState200", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawInitDefaultHardwareState200);
    module.addSymbolExport("yb2cRhagD1I", "sceGnmDrawInitDefaultHardwareState350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawInitDefaultHardwareState350);
    module.addSymbolExport("nF6bFRUBRAU", "sceGnmDispatchInitDefaultHardwareState", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDispatchInitDefaultHardwareState);
    module.addSymbolExport("+AFvOEXrKJk", "sceGnmSetEmbeddedVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetEmbeddedVsShader);
    module.addSymbolExport("X9Omw9dwv5M", "sceGnmSetEmbeddedPsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetEmbeddedPsShader);
    module.addSymbolExport("GGsn7jMTxw4", "sceGnmDrawIndexAuto", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndexAuto);
    module.addSymbolExport("HlTPoZ-oY7Y", "sceGnmDrawIndex", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndex);
    module.addSymbolExport("oYM+YzfCm2Y", "sceGnmDrawIndexOffset", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDrawIndexOffset);
    module.addSymbolExport("0BzLGljcwBo", "sceGnmDispatchDirect", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmDispatchDirect);
    module.addSymbolExport("gAhCn6UiU4Y", "sceGnmSetVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetVsShader);
    module.addSymbolExport("V31V01UiScY", "sceGnmUpdateVsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmUpdateVsShader);
    module.addSymbolExport("bQVd5YzCal0", "sceGnmSetPsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetPsShader);
    module.addSymbolExport("5uFKckiJYRM", "sceGnmSetPsShader350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetPsShader350);
    module.addSymbolExport("4MgRw-bVNQU", "sceGnmUpdatePsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmUpdatePsShader);
    module.addSymbolExport("mLVL7N7BVBg", "sceGnmUpdatePsShader350", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmUpdatePsShader350);
    module.addSymbolExport("KXltnCwEJHQ", "sceGnmSetCsShader", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetCsShader);
    module.addSymbolExport("Kx-h-nWQJ8A", "sceGnmSetCsShaderWithModifier", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmSetCsShaderWithModifier);
    module.addSymbolExport("1qXLHIpROPE", "sceGnmInsertWaitFlipDone", "libSceGnmDriver", "libSceGnmDriver", (void*)&sceGnmInsertWaitFlipDone);
    
    module.addSymbolStub("iBt3Oe00Kvc", "sceGnmFlushGarlic", "libSceGnmDriver", "libSceGnmDriver");
    module.addSymbolStub("W1Etj-jlW7Y", "sceGnmInsertPushMarker", "libSceGnmDriver", "libSceGnmDriver");
    module.addSymbolStub("7qZVNgEu+SY", "sceGnmInsertPopMarker", "libSceGnmDriver", "libSceGnmDriver");
    module.addSymbolStub("jg33rEKLfVs", "sceGnmIsUserPaEnabled", "libSceGnmDriver", "libSceGnmDriver");
}

ComputeQueue compute_queues[MAX_COMPUTE_QUEUES];

s32 PS4_FUNC sceGnmSubmitAndFlipCommandBuffers(u32 cnt, u32** dcb_gpu_addrs, u32* dcb_sizes, u32** ccb_gpu_addrs, u32* ccb_sizes, u32 video_out_handle, u32 buf_idx, u32 flip_mode, u64 flip_arg) {
    log("sceGnmSubmitAndFlipCommandBuffers(cnt=%d, dcb_gpu_addrs=*%p, dcb_sizes=%p, ccb_gpu_addrs=*%p, ccb_sizes=%p, video_out_handle=%d, buf_idx=%d, flip_mode=%d, flip_arg=0x%llx)\n", cnt, dcb_gpu_addrs, dcb_sizes, ccb_gpu_addrs, ccb_sizes, video_out_handle, buf_idx, flip_mode, flip_arg);
    
    for (int i = 0; i < cnt; i++)
        GCN::submitGraphics(dcb_gpu_addrs[i], dcb_sizes[i], ccb_gpu_addrs ? ccb_gpu_addrs[i] : nullptr, ccb_sizes ? ccb_sizes[i] : 0);

    GCN::submitFlip(video_out_handle, buf_idx, flip_arg);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSubmitDone() {
    log("sceGnmSubmitDone()\n");
    
    return SCE_OK;
}

s32 PS4_FUNC sceGnmAddEqEvent(Libs::Kernel::SceKernelEqueue eq, u64 id, void* udata) {
    log("sceGnmAddEqEvent(eq=%p, id=0x%llx, udata=%p)\n", eq, id, udata);

    switch (id) {
    case GCN::EOP_EVENT_ID: GCN::eop_ev_source.addToEventQueue(eq, udata);  break;
    default: Helpers::panic("sceGnmAddEqEvent: unhandled event id 0x%x\n", id);
    }

    return SCE_OK;
}

s32 PS4_FUNC sceGnmMapComputeQueue(u32 pipe_id, u32 queue_id, void* ring_base_addr, u32 ring_size_dw, u32* read_ptr_addr) {
    log("sceGnmMapComputeQueue(pipe_id=%d, queue_id=%d, ring_base_addr=%p, ring_size_dw=0x%x, read_ptr_addr=%p)\n", pipe_id, queue_id, ring_base_addr, ring_size_dw, read_ptr_addr);
    
    const auto qid = queue_id * pipe_id;
    ComputeQueue& queue = compute_queues[qid];
    if (queue.is_mapped) {
        Helpers::panic("Tried to map an already mapped compute queue\n");
    }

    queue = { true, ring_base_addr, ring_size_dw, read_ptr_addr };
    *read_ptr_addr = 0;
    log("Mapped compute queue %d\n", qid + 1);
    return qid + 1;    // Queue id is non-zero
}

s32 PS4_FUNC sceGnmDingDong(u32 queue_id, u32 next_offs_dw) {
    log("sceGnmDingDong(queue_id=%d, next_offs_dw=0x%x)\n", queue_id, next_offs_dw);

    ComputeQueue& queue = compute_queues[queue_id - 1];
    if (!queue.is_mapped) {
        Helpers::panic("sceGnmDingDong on an unmapped queue\n");
    }

    // If a next offset is specified, the queue is executed up to that offset, otherwise until the end of the ring buffer.
    const auto curr_offs = queue.next_offs_dw;
    queue.next_offs_dw = next_offs_dw;

    if (next_offs_dw && next_offs_dw < curr_offs) {
        Helpers::panic("TODO: next_offs_dw < curr_offs");
    }

    // This is the starting offset, aka the "last next offset"
    const u32* cmd_buf_ptr = (u32*)queue.ring_base_addr + curr_offs;                        
    
    // The size is "next offset - current offset", in the case where next_offs_dw is 0 the next offset is the ring buffer size
    const size_t cmd_buf_size_dw = (next_offs_dw ? next_offs_dw : queue.ring_size_dw) - curr_offs;

    // Now that we have the command buffer base and size, submit it to the graphics thread
    GCN::submitCompute((u32*)cmd_buf_ptr, cmd_buf_size_dw, queue_id - 1);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState(u32* buf, u32 size) {
    log("sceGnmDrawInitDefaultHardwareState(buf=%p, size=0x%x) TODO\n", buf, size);

    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 0x100);
    return 0x100;
}

s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState200(u32* buf, u32 size) {
    log("sceGnmDrawInitDefaultHardwareState200(buf=%p, size=0x%x) TODO\n", buf, size);
    
    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 0x100);
    return 0x100;
}

s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState350(u32* buf, u32 size) {
    log("sceGnmDrawInitDefaultHardwareState350(buf=%p, size=0x%x) TODO\n", buf, size);

    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 0x100);
    return 0x100;
}

s32 PS4_FUNC sceGnmDispatchInitDefaultHardwareState(u32* buf, u32 size) {
    log("sceGnmDispatchInitDefaultHardwareState(buf=%p, size=0x%x) TODO\n", buf, size);

    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 0x100);
    return 0x100;
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

s32 PS4_FUNC sceGnmDrawIndex(u32* buf, u32 size, u32 cnt, void* index_buf_ptr, u32 flags, u32 type) {
    log("sceGnmDrawIndex(buf=%p, size=0x%x, cnt=%d, index_buf_ptr=%p, flags=0x%x, type=%d)\n", buf, size, cnt, index_buf_ptr, flags, type);

    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::DrawIndex2, size);
    *buf++ = cnt;
    *buf++ = (u64)index_buf_ptr;
    *buf++ = (u64)index_buf_ptr >> 32;
    *buf++ = cnt;
    *buf++ = 0;
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

s32 PS4_FUNC sceGnmDispatchDirect(u32* buf, u32 size, u32 threads_x, u32 threads_y, u32 threads_z, u32 flags) {
    log("sceGnmDispatchDirect(buf=%p, size=0x%x, threads_x=%d, threads_y=%d, threads_z=%d, flags=0x%x)\n", buf, size, threads_x, threads_y, threads_z, flags);

    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size);
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

// TODO: What is the difference with SetVsShader?
s32 PS4_FUNC sceGnmUpdateVsShader(u32* buf, u32 size, const u32* vs_regs, u32 shader_modifier) {
    log("sceGnmUpdateVsShader(buf=%p, size=0x%x, vs_regs=%p, shader_modifier=0x%x)\n", buf, size, vs_regs, shader_modifier);
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

// Same as sceGnmSetPsShader350?
s32 PS4_FUNC sceGnmSetPsShader(u32* buf, u32 size, const u32* ps_regs) {
    log("sceGnmSetPsShader(buf=%p, size=0x%x, ps_regs=%p)\n", buf, size, ps_regs);

    if (!ps_regs) {
        log("sceGnmSetPsShader: ps_regs is nullptr (TODO)\n");
        *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, 32);
        return SCE_OK;
    }

    log("addr_lo=0x%08x\n", ps_regs[0]);
    log("addr_hi=0x%08x\n", ps_regs[1]);

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

// TODO: What is the difference with SetPsShader?
s32 PS4_FUNC sceGnmUpdatePsShader(u32* buf, u32 size, const u32* ps_regs) {
    log("sceGnmUpdatePsShader(buf=%p, size=0x%x, ps_regs=%p)\n", buf, size, ps_regs);
    log("addr_lo=0x%08x\n", ps_regs[0]);
    log("addr_hi=0x%08x\n", ps_regs[1]);

    if (!ps_regs) {
        Helpers::panic("sceGnmUpdatePsShader: ps_regs is nullptr (TODO)\n");
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

// TODO: What is the difference with SetPsShader350?
s32 PS4_FUNC sceGnmUpdatePsShader350(u32* buf, u32 size, const u32* ps_regs) {
    log("sceGnmUpdatePsShader350(buf=%p, size=0x%x, ps_regs=%p)\n", buf, size, ps_regs);
    log("addr_lo=0x%08x\n", ps_regs[0]);
    log("addr_hi=0x%08x\n", ps_regs[1]);

    if (!ps_regs) {
        Helpers::panic("sceGnmUpdatePsShader: ps_regs is nullptr (TODO)\n");
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

s32 PS4_FUNC sceGnmSetCsShader(u32* buf, u32 size, const u32* cs_regs) {
    log("sceGnmSetCsShader(buf=%p, size=0x%x, cs_regs=%p) TODO\n", buf, size, cs_regs);

    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmSetCsShaderWithModifier(u32* buf, u32 size, const u32* cs_regs, u32 shader_modifier) {
    log("sceGnmSetCsShaderWithModifier(buf=%p, size=0x%x, cs_regs=%p, shader_modifier=0x%x) TODO\n", buf, size, cs_regs, shader_modifier);
    
    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size);
    return SCE_OK;
}

s32 PS4_FUNC sceGnmInsertWaitFlipDone(u32* buf, u32 size, s32 video_out_handle, u32 buf_idx) {
    log("sceGnmInsertWaitFlipDone(buf=%p, size=0x%x, video_out_handle=%d, buf_idx=%d) TODO\n", buf, size, video_out_handle, buf_idx);

    // TODO
    *buf++ = PM4_HEADER_BUILD(GCN::PM4ItOpcode::Nop, size);
    return SCE_OK;
}

} // End namespace SceGnmDriver