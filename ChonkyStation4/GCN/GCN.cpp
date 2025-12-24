#include "GCN.hpp"
#include <GCN/CommandProcessor.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <mutex>
#include <semaphore>
#include <deque>
#include <chrono>


namespace PS4::GCN {

std::deque<RendererCommand> commands;
std::counting_semaphore<256> sem { 0 };
std::mutex mtx;

void gcnThread() {
    using clock = std::chrono::steady_clock;
    const double target_fps = 60.0; // Stubbed for now
    const clock::duration frame_duration = std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / target_fps));
    auto frame_time = clock::now();

    // Initialize renderer
    initVulkan();

    // Initialize event sources
    eop_ev_source.init(EOP_EVENT_ID, 0);    // TODO: Properly set filter
    
    initialized = true;

    RendererCommand cmd;
    while (true) {
        // Wait until we have work to do.
        sem.acquire();

        {
            // Acquire command queue lock
            std::scoped_lock lk(mtx);
            // Fetch command from the queue
            cmd = commands.front();
            commands.pop_front();
        }

        // Process the command
        switch (cmd.type) {
        case CommandType::SubmitCommandBuffers: {
            GCN::processCommands(cmd.dcb, cmd.dcb_size, cmd.ccb, cmd.ccb_size);
            break;
        }

        case CommandType::Flip: {
            // Set buffer label
            u64* buf_label;
            OS::Libs::SceVideoOut::sceVideoOutGetBufferLabelAddress(cmd.video_out_handle, (void**)&buf_label);
            buf_label[cmd.buf_idx] = 1;
            renderer->flip();

            // Signal SceVideoOut port event queues
            auto port = PS4::OS::find<OS::Libs::SceVideoOut::SceVideoOutPort>(cmd.video_out_handle);
            if (!port) {
                Helpers::panic("gcn thread flip: handle %d does not exist\n", cmd.video_out_handle);
            }
            port->signalFlip(cmd.flip_arg);
            buf_label[cmd.buf_idx] = 0;

            // Frame limiter
            frame_time += frame_duration;
            auto now = clock::now();
            if (now < frame_time) {
                std::this_thread::sleep_until(frame_time);
            }
            else frame_time = now;
            break;
        }
        }
    }
}

void submitRendererCommand(RendererCommand cmd) {
    {
        // Acquire command queue lock
        std::scoped_lock lk(mtx);
        // Push command
        commands.push_back(cmd);
    }
    // Increment semaphore
    sem.release();
}

void submitCommandBuffers(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size) {
    submitRendererCommand({ CommandType::SubmitCommandBuffers, dcb, dcb_size, ccb, ccb_size });
}

void submitFlip(u32 video_out_handle, u32 buf_idx, u64 flip_arg) {
    submitRendererCommand({ CommandType::Flip, .video_out_handle = video_out_handle, .buf_idx = buf_idx, .flip_arg = flip_arg });
}

}   // End namespace PS4::GCN