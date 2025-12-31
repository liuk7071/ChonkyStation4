#pragma once

#include <Common.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <GCN/Backends/Vulkan/VulkanRenderer.hpp>
#include <atomic>


namespace PS4::GCN {

static constexpr u32 EOP_EVENT_ID = 0x40;

enum class CommandType {
    SubmitGraphics,
    SubmitCompute,
    Flip
};

struct RendererCommand {
    CommandType type;

    // For SubmitGraphics
    u32* dcb = nullptr;     // Reused for SubmitCompute too
    size_t dcb_size = 0;    // Reused for SubmitCompute too
    u32* ccb = nullptr;
    size_t ccb_size = 0;

    // For SubmitCompute
    u32 queue_id = 0;

    // For Flip
    u32 video_out_handle = 0;
    u32 buf_idx = -1;
    u64 flip_arg = 0;
};

inline std::atomic<bool> initialized = false;
inline std::unique_ptr<Renderer> renderer;

// Event sources
inline OS::Libs::Kernel::EventSource eop_ev_source;

void gcnThread();
void submitGraphics(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size);
void submitCompute(u32* cb, size_t cb_size, u32 queue_id);
void submitFlip(u32 video_out_handle, u32 buf_idx, u64 flip_arg);

inline void initVulkan() {
    renderer = std::make_unique<Vulkan::VulkanRenderer>();
    renderer->init();
}

}   // End namespace PS4::GCN