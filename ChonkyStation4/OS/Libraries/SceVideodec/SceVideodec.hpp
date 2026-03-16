#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceVideodec {

void init(Module& module);

using SceVideodec2ComputeQueue = void*;
using SceKernelCpumask = u64;

struct SceVideodecConfigInfo {
    size_t this_size;
    u32 codec_type;
    u32 profile;
    u32 max_level;
    s32 max_frame_width;
    s32 max_frame_height;
    s32 max_dpb_frame_count;
    u64 videodec_flags;
};

struct SceVideodecResourceInfo {
    size_t thisSize;
    size_t cpu_mem_size;
    void* cpu_mem;
    size_t cpu_gpu_mem_size;
    void* cpu_gpu_mem;
    size_t max_framebuffer_size;
    u32 framebuffer_alignment;
};

struct SceVideodec2DecoderConfigInfo {
    size_t this_size;
    u32 resource_type;
    u32 codec_type;
    u32 profile;
    u32 max_level;
    s32 max_frame_width;
    s32 max_frame_height;
    s32 maxDpbFrameCount;
    u32 decode_pipeline_depth;
    SceVideodec2ComputeQueue compute_queue;
    SceKernelCpumask cpu_affinity_mask;
    s32 cpu_thread_priority;
    bool optimize_progressive_video;
    bool check_mem_type;
    u8 reserved0;
    u8 reserved1;
    void* extra_config_info;
};

struct SceVideodec2DecoderMemoryInfo {
    size_t this_size;
    size_t cpu_mem_size;
    void* cpu_mem;
    size_t gpu_mem_size;
    void* gpu_mem;
    size_t cpu_gpu_mem_size;
    void* cpu_gpu_mem;
    size_t max_framebuffer_size;
    u32 framebuffer_alignment;
    u32 reserved0;
};

// libSceVideodec
s32 PS4_FUNC sceVideodecQueryResourceInfo(const SceVideodecConfigInfo* cfg_info, SceVideodecResourceInfo* resource_info);

// libSceVideodec2
s32 PS4_FUNC sceVideodec2QueryDecoderMemoryInfo(const SceVideodec2DecoderConfigInfo* cfg_info, SceVideodec2DecoderMemoryInfo* mem_info);

}   // End namespace PS4::OS::Libs::SceVideodec