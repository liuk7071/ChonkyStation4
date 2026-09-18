#pragma once

#include <Common.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <GCN/Backends/Vulkan/VulkanRenderer.hpp>
#include <co.hpp>
#include <atomic>


namespace PS4::OS::Libs::SceGnmDriver {

struct ComputeQueue;

}   // End namespace PS4::OS::Libs::SceGnmDriver

namespace PS4::GCN {

static constexpr u32 EOP_EVENT_ID = 0x40;

inline std::atomic<u64> global_flip_counter = 0;

// Renderer commands

enum class CommandType {
    SubmitGraphics,
    SubmitCompute,
    Flip
};

struct ComputeQueue;
struct RendererCommand {
    CommandType type;

    // For SubmitGraphics
    u32* dcb = nullptr;     // Reused for SubmitCompute too
    size_t dcb_size = 0;    // Reused for SubmitCompute too
    u32* ccb = nullptr;
    size_t ccb_size = 0;

    // For SubmitCompute
    ComputeQueue* queue = nullptr;

    // For Flip
    u32 video_out_handle = 0;
    u32 buf_idx = -1;
    u64 flip_arg = 0;

    // If the copy command buffers setting is enabled
    u8* dcb_buf = nullptr;
    u8* ccb_buf = nullptr;
    std::vector<u8*> indirect_bufs;
};

// Async compute

static constexpr s32 MAX_COMPUTE_QUEUES_PER_PIPE = 8;
static constexpr s32 MAX_COMPUTE_PIPES = 7;
static constexpr s32 MAX_COMPUTE_QUEUES = MAX_COMPUTE_QUEUES_PER_PIPE * MAX_COMPUTE_PIPES;

struct ComputeQueue {
    bool is_mapped = false;
    u32 qid = 0;
    void* ring_base_addr = nullptr;
    u32 ring_size_dw = 0;
    u32* read_ptr_addr = nullptr;
    u32 next_offs_dw = 0;

    std::mutex mtx;
    std::deque<RendererCommand> commands;
    co::thread* co = nullptr;
    bool co_done = true;

    std::unique_lock<std::mutex> getLock() {
        return std::move(std::unique_lock<std::mutex>(mtx));
    }
};

inline ComputeQueue compute_queues[MAX_COMPUTE_QUEUES];

inline std::atomic<bool> initialized = false;
inline std::unique_ptr<Renderer> renderer;

// Event sources
inline OS::Libs::Kernel::EventSource eop_ev_source;

void gcnThread();
bool processAsyncCompute();
void submitGraphics(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size);
void submitCompute(u32* cb, size_t cb_size, ComputeQueue* queue);
void submitFlip(u32 video_out_handle, u32 buf_idx, u64 flip_arg);
bool isCommandProcessorIdle();

inline void initVulkan() {
    renderer = std::make_unique<Vulkan::VulkanRenderer>();
    renderer->init();
}

}   // End namespace PS4::GCN