#include "GCN.hpp"
#include <Configuration.hpp>
#include <GCN/CommandProcessor.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>
#include <GCN/PM4.hpp>
#include <mutex>
#include <semaphore>
#include <deque>
#include <chrono>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif


namespace PS4::GCN {

std::deque<RendererCommand> commands;
std::counting_semaphore<256> sem { 0 };
std::mutex mtx;
int prev_flip_idx = -1;
int curr_asc = 0;

void gcnThread() {
#ifdef _WIN32
    SetThreadDescription(GetCurrentThread(), L"[Emu] GCN Thread");
#endif

    using clock = std::chrono::steady_clock;
    const double target_fps = Configuration::fps_limit;
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
            if (Configuration::copy_command_buffers) {
                delete[] cmd.dcb_buf;
                delete[] cmd.ccb_buf;
            }
            break;
        }

        case CommandType::Flip: {
            auto port = PS4::OS::find<OS::Libs::SceVideoOut::SceVideoOutPort>(cmd.video_out_handle);
            if (!port) {
                Helpers::panic("gcn thread flip: handle %d does not exist\n", cmd.video_out_handle);
            }
            
            if (cmd.buf_idx == -1) {
                Helpers::panic("Flip: buf_idx is -1\n");
                port->signalFlip(cmd.flip_arg);
                break;
            }

            // Set buffer label
            u64* buf_label;
            OS::Libs::SceVideoOut::sceVideoOutGetBufferLabelAddress(cmd.video_out_handle, (void**)&buf_label);
            buf_label[cmd.buf_idx] = 1;
            renderer->flip(&OS::Libs::SceVideoOut::bufs[cmd.buf_idx]);
            global_flip_counter++;

            if (Configuration::is_vsh)
                OS::Libs::SceVideoOut::bufs[cmd.buf_idx].base = OS::Libs::SceVideoOut::sce_composite_color_target_addr;

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
    int i = 0;
    while (true) {
        auto& queue = compute_queues[curr_asc++];
        curr_asc %= MAX_COMPUTE_QUEUES;

        // Give up if we went a full round without active queues
        if (i++ == MAX_COMPUTE_QUEUES) return false;

        {
            auto lk = queue.getLock();

            if (!queue.is_mapped) continue;

            // If this queue is done executing and there are no pending commands (idle)
            if (queue.co_done && queue.commands.empty()) continue;

            if (queue.co_done) {
                RendererCommand cmd;
                {
                    if (queue.commands.empty())
                        Helpers::panic("processAsyncCompute: unreachable\n");   // We check earlier that it's not empty

                    // Fetch command from the queue
                    cmd = queue.commands.front();
                    queue.commands.pop_front();
                }

                queue.co_done = false;

                if (!queue.co)
                    queue.co = new co::thread();

                queue.co->reset([=]() {
                    GCN::processCommands(cmd.dcb, cmd.dcb_size, nullptr, 0, cmd.queue);
                    cmd.queue->co_done = true;
                    if (Configuration::copy_command_buffers) {
                        delete[] cmd.dcb_buf;
                        for (auto& buf : cmd.indirect_bufs)
                            delete[] buf;
                    }
                    co::active().get_parent().switch_to();
                });
            }
        }

        //printf("switching to asc %d\n", queue.qid);
        queue.co->switch_to();
        return true;
    }

    Helpers::panic("processAsyncCompute: unreachable\n");
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
    RendererCommand cmd = { CommandType::SubmitGraphics, dcb, dcb_size, ccb, ccb_size };

    if (Configuration::copy_command_buffers) {
        cmd.dcb_buf = new u8[dcb_size];
        cmd.ccb_buf = new u8[ccb_size];
        std::memcpy(cmd.dcb_buf, dcb, dcb_size);
        std::memcpy(cmd.ccb_buf, ccb, ccb_size);
        cmd.dcb = (u32*)cmd.dcb_buf;
        cmd.ccb = (u32*)cmd.ccb_buf;
    }

    submitRendererCommand(cmd);
}

void submitCompute(u32* cb, size_t cb_size, ComputeQueue* queue) {
    RendererCommand cmd = { CommandType::SubmitCompute, cb, cb_size, .queue = queue };

    if (Configuration::copy_command_buffers) {
        cmd.dcb_buf = new u8[cb_size];
        std::memcpy(cmd.dcb_buf, cb, cb_size);
        cmd.dcb = (u32*)cmd.dcb_buf;

        // Copy and patch indirect buffers
        /*for (u32* ptr = cmd.dcb; (u8*)ptr < (u8*)cmd.dcb + cmd.dcb_size; ) {
            PM4Header* pkt = (PM4Header*)ptr;
            u32* args = ptr;
            args++;

            if (pkt->type == 0) {
                ptr++;
                continue;
                Helpers::panic("PM4 type 0 packet\n");
            }
            else if (pkt->type == 1) {
                Helpers::panic("PM4 type 1 packet\n");
            }
            else if (pkt->type == 2) {
                printf("Encountered type 2 packet\n");
                ptr++;
                continue;
            }

            switch ((PM4ItOpcode)(u32)pkt->opcode) {
            case PM4ItOpcode::IndirectBuffer: {
                union IndirectBuffer1 {
                    u32 raw;
                    BitField<0, 16, u32> addr_hi;
                };

                union IndirectBuffer2 {
                    u32 raw;
                    BitField<0, 20, u32> size;
                    BitField<20, 1, u32> chain;
                    BitField<24, 8, u32> vmid;
                };

                u32* addr_lo_ptr = args;
                const u32 addr_lo = *args++;

                IndirectBuffer1* d1_ptr = (IndirectBuffer1*)args;
                const IndirectBuffer1 d1 = { .raw = *args++ };

                const IndirectBuffer2 d2 = { .raw = *args++ };
                const u32* ptr = (u32*)(addr_lo | ((u64)d1.addr_hi << 32));

                auto& buf = cmd.indirect_bufs.emplace_back();
                buf = new u8[d2.size * sizeof(u32)];
                std::memcpy(buf, ptr, d2.size * sizeof(u32));

                d1_ptr->addr_hi = (uptr)ptr >> 32;
                *addr_lo_ptr = (uptr)ptr & 0xffffffff;
                break;
            }

            }

            ptr += pkt->count + 2;
        }*/
    }

    //printf("submitted asc for queue %d\n", queue->qid);

    // queue lock should be held by the caller
    queue->commands.push_back(cmd);
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