#pragma once

#include <Common.hpp>
#include <semaphore>
#include <memory>
#include <mutex>
#include <atomic>


namespace PS4::OS::Libs::Kernel {

using sema = std::counting_semaphore<INT32_MAX>;

struct Semaphore {
    Semaphore(s32 init_count, s32 max_count) : init_count(init_count), max_count(max_count) {
        std_sema = std::make_unique<sema>(init_count);
        counter = init_count;
        waiters = 0;
    }

    std::string name;
    s32 init_count;
    s32 max_count;
    std::atomic<s32> counter;
    std::unique_ptr<sema> std_sema;
    std::mutex mtx;
    std::atomic<s32> waiters;
    std::atomic<bool> cancelled = false;
    std::atomic<s32> woke_from_cancel = 0;

    void signal(s32 count, bool is_from_cancel = false);
    bool wait(s32 count, u32 timeout, bool& was_cancelled);
    bool poll(s32 count);
    int cancel(s32 new_count);
};

struct SceKernelSemaOptParam;
using SceKernelSema = Semaphore*;

s32 PS4_FUNC sceKernelCreateSema(SceKernelSema* sem, const char* name, u32 attr, s32 init_count, s32 max_count, const SceKernelSemaOptParam* opt_param);
s32 PS4_FUNC sceKernelSignalSema(SceKernelSema sem, s32 count);
s32 PS4_FUNC sceKernelWaitSema(SceKernelSema sem, s32 count, u32* timeout);
s32 PS4_FUNC sceKernelPollSema(SceKernelSema sem, s32 count);
s32 PS4_FUNC sceKernelCancelSema(SceKernelSema sem, s32 set_count, s32* n_released_threads);

s32 PS4_FUNC kernel_sem_init(SceKernelSema* sem, s32 pshared, u32 value);
s32 PS4_FUNC kernel_sem_post(SceKernelSema* sem);
s32 PS4_FUNC kernel_sem_wait(SceKernelSema* sem);
s32 PS4_FUNC kernel_sem_timedwait(SceKernelSema* sem, const SceKernelTimespec* time);
s32 PS4_FUNC kernel_sem_getvalue(SceKernelSema* sem, s32* val);

};  // End namespace PS4::OS::Libs::Kernel