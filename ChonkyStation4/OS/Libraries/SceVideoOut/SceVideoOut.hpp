#pragma once

#include <Common.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceVideoOut {

void init(Module& module);


static constexpr s32 SCE_VIDEO_OUT_MAX_DISPLAY_BUFFERS = 16;
static constexpr s32 SCE_USER_SERVICE_USER_ID_SYSTEM = 255;
static constexpr s32 SCE_VIDEO_OUT_BUS_TYPE_MAIN = 0;

struct SceVideoOutBuffer {
    s32 group_idx = -1;
    void* addr_left = nullptr;
    void* addr_right = nullptr;
};

struct SceVideoOutPort : SceObj {
    u64 buffer_labels[SCE_VIDEO_OUT_MAX_DISPLAY_BUFFERS];
};

s32 PS4_FUNC sceVideoOutOpen(s32 uid, s32 bus_type, s32 idx, const void* param);
s32 PS4_FUNC sceVideoOutGetBufferLabelAddress(s32 handle, void** label_addr);
s32 PS4_FUNC sceVideoOutSetFlipRate(s32 handle, s32 rate);

}   // End namespace PS4::OS::Libs::SceVideoOut