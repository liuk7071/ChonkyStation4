#pragma once

#include <Common.hpp>


class Module;
struct TSharp;

namespace PS4::OS::Libs::SceComposite {

void init(Module& module);

void* PS4_FUNC sceCompositorGetSystemAddress();
void* PS4_FUNC sceCompositorGetVideoAddress();
s32 PS4_FUNC sceCompositorGetRenderTargetResolution(s16* width, s16* height);
s32 PS4_FUNC sceCompsoitorGetGpuClock(u64* gpu_clock);
s32 PS4_FUNC sceCompositorWaitPostEvent();
s32 PS4_FUNC sceCompositorSetGnmContextCommand(u32* dcb_gpu_addr, u32 dcb_size, u32* ccb_gpu_addr, u32 ccb_size);
s32 PS4_FUNC sceCompositorSetFlipCommand();
s32 PS4_FUNC sceCompositorSetCompositeCanvasCommandInC(void* unk1, TSharp* tsharp);
s32 PS4_FUNC sceCompositorSetPostEventCommand(void* cmd);

}   // End namespace PS4::OS::Libs::SceComposite