#include "SceZlib.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <miniz.h>
#include <mutex>
#include <semaphore>
#include <deque>
#include <atomic>
#include <thread>


namespace PS4::OS::Libs::SceZlib {

MAKE_LOG_FUNCTION(log, lib_sceZlib);

void init(Module& module) {
    module.addSymbolExport("m1YErdIXCp4", "sceZlibInitialize", "libSceZlib", "libSceZlib", (void*)&sceZlibInitialize);
    module.addSymbolExport("TLar1HULv1Q", "sceZlibInflate", "libSceZlib", "libSceZlib", (void*)&sceZlibInflate);
    module.addSymbolExport("uB8VlDD4e0s", "sceZlibWaitForDone", "libSceZlib", "libSceZlib", (void*)&sceZlibWaitForDone);
    module.addSymbolExport("2eDcGHC0YaM", "sceZlibGetResult", "libSceZlib", "libSceZlib", (void*)&sceZlibGetResult);
}

std::thread inflate_thread;
std::counting_semaphore queue_sema { 0 };
std::mutex queue_mtx;

struct InflateTask {
    u64 req_id;
    const void* src;
    u32 src_len;
    void* dst;
    u32 dst_len;

    u32 out_len = 0;
    std::atomic<bool> completed = false;
    std::binary_semaphore sema { 0 };
};
std::deque<std::shared_ptr<InflateTask>> queue;

static bool initialized = false;
s32 PS4_FUNC sceZlibInitialize(const void* buffer, size_t length) {
    log("sceZlibInitialize(buffer=%p, length=%lld)\n", buffer, length);

    if (buffer || length) {
        log("sceZlibInitialize: argument error\n");
        return SCE_ZLIB_ERROR_INVALID;
    }

    if (initialized) {
        log("sceZlibInitialize: already initialized\n");
        return SCE_ZLIB_ERROR_ALREADY_INITIALIZED;
    }

    inflate_thread = std::thread([]() {
        while (true) {
            // Wait for a task
            queue_sema.acquire();

            std::shared_ptr<InflateTask> task;
            {
                const std::unique_lock<std::mutex> lk(queue_mtx);
                
                // Find a task that was not completed
                auto it = std::find_if(queue.begin(), queue.end(), [&](const std::shared_ptr<InflateTask>& task) {
                    return !task->completed;
                });

                if (it == queue.end())
                    Helpers::panic("inflate_thread: semaphore was signalled but no task was found\n");

                task = *it;
            }

            // Inflate
            unsigned long out_len = task->dst_len;
            auto res = mz_uncompress((u8*)task->dst, &out_len, (u8*)task->src, task->src_len);
            if (res != Z_OK) {
                Helpers::panic("inflate_thread: uncompress error %d\n", res);
            }

            // Max allowed size after decompression is 64kb
            if (out_len > 64_KB) {
                Helpers::panic("inflate_thread: decompressed size is > 64 kb (%d bytes, %d kb)\n", out_len, out_len / 1_KB);
            }

            // Flag task as completed. It will be removed from the queue once the guest calls sceZlibGetResult
            task->out_len = out_len;
            task->completed = true;
            task->sema.release();
        }
    });

    inflate_thread.detach();
    initialized = true;
    return SCE_OK;
}

std::atomic<u64> next_req_id = 0x1;
s32 PS4_FUNC sceZlibInflate(const void* src, u32 src_len, void* dst, u32 dst_len, u64* req_id) {
    log("sceZlibInflate(src=%p, src_len=%d, dest=%p, dest_len=%d, req_id=*%p)\n", src, src_len, dst, dst_len, req_id);
    
    const auto new_req_id = next_req_id++;
    *req_id = new_req_id;

    const std::unique_lock<std::mutex> lk(queue_mtx);
    auto task = std::make_shared<InflateTask>(new_req_id, src, src_len, dst, dst_len);
    queue.push_back(std::move(task));
    queue_sema.release();
    return SCE_OK;
}

s32 PS4_FUNC sceZlibWaitForDone(u64* req_id, u32* timeout) {
    log("sceZlibWaitForDone(req_id=*%p, timeout=*%p)\n", req_id, timeout);
    
    if (!req_id) {
        log("sceZlibWaitForDone: req_id is null\n");
        return SCE_ZLIB_ERROR_INVALID;
    }

    if (timeout)
        printf("TODO: sceZlibWaitForDone timeout\n");

    log("req_id=%lld\n", *req_id);
    
    // Find task
    std::shared_ptr<InflateTask> task;
    {
        const std::unique_lock<std::mutex> lk(queue_mtx);

        auto it = std::find_if(queue.begin(), queue.end(), [&](const std::shared_ptr<InflateTask>& task) {
            return task->req_id == *req_id;
        });

        if (it == queue.end())
            Helpers::panic("sceZlibWaitForDone: task %lld not found\n", *req_id);

        task = *it;
    }

    if (!task->completed)
        task->sema.acquire();

    return SCE_OK;
}

s32 PS4_FUNC sceZlibGetResult(u64 req_id, u32* dst_len, s32* status) {
    log("sceZlibGetResult(req_id=*%p, dst_len=*%p, status=*%p)\n", req_id, dst_len, status);

    // Find task
    const std::unique_lock<std::mutex> lk(queue_mtx);

    auto it = std::find_if(queue.begin(), queue.end(), [&](const std::shared_ptr<InflateTask>& task) {
        return task->req_id == req_id;
    });

    if (it == queue.end())
        Helpers::panic("sceZlibGetResult: task %lld not found\n", req_id);

    auto task = *it;
    if (!task->completed)
        Helpers::panic("sceZlibGetResult: task was not complete\n");

    *dst_len = task->out_len;
    *status = 0;
    queue.erase(it);
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceZlib