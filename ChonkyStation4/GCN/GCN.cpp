#include "GCN.hpp"
#include <GCN/CommandProcessor.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>
#include <mutex>
#include <semaphore>
#include <deque>
#include <chrono>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <co.hpp>


namespace PS4::GCN {

std::deque<RendererCommand> commands;
std::deque<RendererCommand> asc_commands;
std::counting_semaphore<256> sem { 0 };
std::mutex mtx;
std::mutex asc_mtx;
int prev_flip_idx = -1;
co::thread* asc_co;
bool asc_co_done = true;

void gcnThread() {
#ifdef _WIN32
    SetThreadDescription(GetCurrentThread(), L"[Emu] GCN Thread");
#endif

    using clock = std::chrono::steady_clock;
    const double target_fps = 60.0; // Stubbed for now
    const clock::duration frame_duration = std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / target_fps));
    auto frame_time = clock::now();

    // Initialize renderer
    initVulkan();

    // Initialize event sources
    eop_ev_source.init(EOP_EVENT_ID, -14);
    
    GCN::initCommandProcessor();
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
        case CommandType::SubmitGraphics: {
            processAsyncCompute();

            GCN::processCommands(cmd.dcb, cmd.dcb_size, cmd.ccb, cmd.ccb_size, nullptr);
            break;
        }

        //case CommandType::SubmitCompute: {
        //    GCN::processCommands(cmd.dcb, cmd.dcb_size, nullptr, 0, cmd.queue);
        //    break;
        //}

        case CommandType::Flip: {
            auto port = PS4::OS::find<OS::Libs::SceVideoOut::SceVideoOutPort>(cmd.video_out_handle);
            if (!port) {
                Helpers::panic("gcn thread flip: handle %d does not exist\n", cmd.video_out_handle);
            }
            
            //if (cmd.buf_idx == -1) {
            //    port->signalFlip(cmd.flip_arg);
            //    break;
            //}

            // Set buffer label
            u64* buf_label;
            OS::Libs::SceVideoOut::sceVideoOutGetBufferLabelAddress(cmd.video_out_handle, (void**)&buf_label);
            buf_label[cmd.buf_idx] = 1;
            renderer->flip(&OS::Libs::SceVideoOut::bufs[cmd.buf_idx]);

            // Signal SceVideoOut port event queues
            port->signalFlip(cmd.flip_arg);
            if (prev_flip_idx >= 0)
                buf_label[prev_flip_idx] = 0;
            prev_flip_idx = cmd.buf_idx;

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

// Returns false if there are no compute queues to execute
bool processAsyncCompute() {
    if (asc_co_done) {
        RendererCommand cmd;
        {
            // Acquire command queue lock
            std::scoped_lock lk(asc_mtx);

            if (asc_commands.empty())
                return false;

            // Fetch command from the queue
            cmd = asc_commands.front();
            asc_commands.pop_front();
        }

        asc_co_done = false;

        if (!asc_co)
            asc_co = new co::thread();

        asc_co->reset([=]() {
            GCN::processCommands(cmd.dcb, cmd.dcb_size, nullptr, 0, cmd.queue);
            asc_co_done = true;
            co::active().get_parent().switch_to();
        });
    }
    
    asc_co->switch_to();
    return true;
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

void submitGraphics(u32* dcb, size_t dcb_size, u32* ccb, size_t ccb_size) {
    submitRendererCommand({ CommandType::SubmitGraphics, dcb, dcb_size, ccb, ccb_size });
}

void submitCompute(u32* cb, size_t cb_size, OS::Libs::SceGnmDriver::ComputeQueue* queue) {
    {
        // Acquire command queue lock
        std::scoped_lock lk(asc_mtx);
        // Push command
        asc_commands.push_back({ CommandType::SubmitCompute, cb, cb_size, .queue = queue });
    }
}

void submitFlip(u32 video_out_handle, u32 buf_idx, u64 flip_arg) {
    submitRendererCommand({ CommandType::Flip, .video_out_handle = video_out_handle, .buf_idx = buf_idx, .flip_arg = flip_arg });
}

bool isCommandProcessorIdle() {
    size_t size;
    {
        // Acquire command queue lock
        std::scoped_lock lk(mtx);
        size = commands.size();
    }
    return size == 0;
}

}   // End namespace PS4::GCN