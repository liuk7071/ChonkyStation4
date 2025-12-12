#pragma once

#include <Common.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>


class Module;

namespace PS4::OS::Libs::SceGnmDriver {

void init(Module& module);

s32 PS4_FUNC sceGnmSubmitAndFlipCommandBuffers(u32 cnt, u32** dcb_gpu_addrs, u32* dcb_sizes, u32** ccb_gpu_addrs, u32* ccb_sizes, u32 video_out_handle, u32 buf_idx, u32 flip_mode, u64 flip_arg);
s32 PS4_FUNC sceGnmSubmitDone();
s32 PS4_FUNC sceGnmAddEqEvent(Libs::Kernel::SceKernelEqueue eq, u64 id, void* udata);
s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState200(u32* buf, u32 size);
s32 PS4_FUNC sceGnmDrawInitDefaultHardwareState350(u32* buf, u32 size);
s32 PS4_FUNC sceGnmSetEmbeddedVsShader(u32* buf, u32 size, u32 shader_id, u32 shader_modifier);
s32 PS4_FUNC sceGnmSetEmbeddedPsShader(u32* buf, u32 size, u32 shader_id, u32 shader_modifier);
s32 PS4_FUNC sceGnmDrawIndexAuto(u32* buf, u32 size, u32 cnt, u32 flags);
s32 PS4_FUNC sceGnmDrawIndex(u32* buf, u32 size, u32 cnt, void* index_buf_ptr, u32 flags, u32 type);
s32 PS4_FUNC sceGnmDrawIndexOffset(u32* buf, u32 size, u32 start, u32 cnt, u32 flags);
s32 PS4_FUNC sceGnmDispatchDirect(u32* buf, u32 size, u32 threads_x, u32 threads_y, u32 threads_z, u32 flags);
s32 PS4_FUNC sceGnmSetVsShader(u32* buf, u32 size, const u32* vs_regs, u32 shader_modifier);
s32 PS4_FUNC sceGnmUpdateVsShader(u32* buf, u32 size, const u32* vs_regs, u32 shader_modifier);
s32 PS4_FUNC sceGnmSetPsShader(u32* buf, u32 size, const u32* ps_regs);
s32 PS4_FUNC sceGnmSetPsShader350(u32* buf, u32 size, const u32* ps_regs);
s32 PS4_FUNC sceGnmUpdatePsShader(u32* buf, u32 size, const u32* ps_regs);
s32 PS4_FUNC sceGnmSetCsShader(u32* buf, u32 size, const u32* cs_regs);
s32 PS4_FUNC sceGnmSetCsShaderWithModifier(u32* buf, u32 size, const u32* cs_regs, u32 shader_modifier);
s32 PS4_FUNC sceGnmInsertWaitFlipDone(u32* buf, u32 size, s32 video_out_handle, u32 buf_idx);

}   // End namespace PS4::OS::Libs::SceGnmDriver