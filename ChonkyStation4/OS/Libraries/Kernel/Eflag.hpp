#pragma once

#include <Common.hpp>
#include <list>
#include <condition_variable>
#include <mutex>
#include <semaphore>


namespace PS4::OS::Libs::Kernel {

struct Eflag {
    std::string name;
    u64 bitptn;
    std::mutex mtx;

    struct Waiter {
        u64 bitptn;
        u32 wait_mode;
        u64 result = 0;
        bool check_done = false;
        std::condition_variable cv;
        std::mutex mtx;
    };

    bool is_multi = false;
    std::list<std::shared_ptr<Waiter>> waiters;

    bool checkCond(const Eflag::Waiter& waiter);
    bool maybeWakeup(Waiter& waiter);
    void set(u64 bitptn);
    void clear(u64 bitptn);
    bool wait(u64 bitptn, u32 wait_mode, u64& result);
    bool poll(u64 bitptn, u32 wait_mode, u64& result);
};

struct SceKernelEventFlagOptParam;
using SceKernelEventFlag = Eflag*;

s32 PS4_FUNC sceKernelCreateEventFlag(SceKernelEventFlag* ef, const char* name, u32 attr, u64 init_ptn, const SceKernelEventFlagOptParam* opt_param);
s32 PS4_FUNC sceKernelSetEventFlag(SceKernelEventFlag ef, u64 bitptn);
s32 PS4_FUNC sceKernelClearEventFlag(SceKernelEventFlag ef, u64 bitptn);
s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout);
s32 PS4_FUNC sceKernelPollEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result);

};  // End namespace PS4::OS::Libs::Kernel