#pragma once

#include <Common.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <GCN/Backends/Vulkan/VulkanRenderer.hpp>
#include <atomic>


namespace PS4::GCN {


enum class CommandType {
    SubmitCommandBuffers,
    Flip
};

struct RendererCommand {
    CommandType type;

    // For SubmitCommandBuffers
    u32* dcb = nullptr;
    size_t dcb_size = 0;
    u32* ccb = nullptr;
    size_t ccb_size = 0;

    // For Flip
    u32 video_out_handle;
    u64 flip_arg;
};

inline std::atomic<bool> initialized = false;
inline std::unique_ptr<Renderer> renderer;

void gcnThread();
void submitCommandBuffers(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size);
void submitFlip(u32 video_out_handle, u64 flip_arg);

inline void initVulkan() {
    renderer = std::make_unique<VulkanRenderer>();
    renderer->init();
}

}   // End namespace PS4::GCN