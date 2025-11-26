#pragma once

#include <Common.hpp>
#include <OS/SceObj.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>


class Module;

namespace PS4::OS::Libs::SceVideoOut {

void init(Module& module);

static constexpr s32 SCE_VIDEO_OUT_MAX_DISPLAY_BUFFERS = 16;
static constexpr s32 SCE_USER_SERVICE_USER_ID_SYSTEM = 255;
static constexpr s32 SCE_VIDEO_OUT_BUS_TYPE_MAIN = 0;

static constexpr s32 SCE_VIDEO_OUT_FLIP_EVENT_ID = 0x6;

struct SceVideoOutBuffer {
    s32 group_idx = -1;
    void* addr_left = nullptr;
    void* addr_right = nullptr;
};

struct SceVideoOutBufferAttribute {
	s32 pixel_format;
	s32 tiling_mode;
	s32 aspect_ratio;
	u32 width;
	u32 height;
	u32 pitch_in_pixels;
	u32 option;
	u32 _reserved0;
	u64 _reserved1;
};

struct SceVideoOutFlipStatus {
	u64 count;
	u64 process_time;
	u64 tsc;
	s64 flip_arg;
	u64 submit_tsc;
	u64 _reserved0;
	s32 gc_queue_num;
	s32 n_pending;
	s32 curr_buf_idx;
	u32 _reserved1;
};

struct SceVideoOutResolutionStatus {
	u32 full_width;
	u32 full_height;
	u32 pane_width;
	u32 pane_height;
	u64 refresh_rate;
	float screen_size_in_inch;
	u16 flags;
	u16 _reserved0;
	u32 _reserved1[3];
};

struct SceVideoOutPort : SceObj {
    u64 buffer_labels[SCE_VIDEO_OUT_MAX_DISPLAY_BUFFERS];
    SceVideoOutFlipStatus flip_status;
    std::vector<Kernel::SceKernelEqueue> flip_eqs;
	SceVideoOutResolutionStatus resolution_status;
};

s32 PS4_FUNC sceVideoOutOpen(s32 uid, s32 bus_type, s32 idx, const void* param);
s32 PS4_FUNC sceVideoOutGetBufferLabelAddress(s32 handle, void** label_addr);
s32 PS4_FUNC sceVideoOutSetFlipRate(s32 handle, s32 rate);
s32 PS4_FUNC sceVideoOutAddFlipEvent(Kernel::SceKernelEqueue eq, s32 handle, void* udata);
s32 PS4_FUNC sceVideoOutRegisterBuffers(s32 handle, s32 start_idx, void** addrs, s32 n_bufs, SceVideoOutBufferAttribute* attrib);
s32 PS4_FUNC sceVideoOutSetBufferAttribute(SceVideoOutBufferAttribute* attrib, u32 pixel_format, u32 tiling_mode, u32 aspect_ratio, u32 width, u32 height, u32 pitch_in_pixels);
s32 PS4_FUNC sceVideoOutSubmitChangeBufferAttribute(s32 handle, s32 idx, SceVideoOutBufferAttribute* attrib);
s32 PS4_FUNC sceVideoOutSubmitFlip(s32 handle, s32 buf_idx, s32 flip_mode, s64 flip_arg);
s32 PS4_FUNC sceVideoOutGetFlipStatus(s32 handle, SceVideoOutFlipStatus* status);
s32 PS4_FUNC sceVideoOutGetResolutionStatus(s32 handle, SceVideoOutResolutionStatus* status);

}   // End namespace PS4::OS::Libs::SceVideoOut