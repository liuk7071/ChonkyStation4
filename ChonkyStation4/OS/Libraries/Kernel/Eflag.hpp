#pragma once

#include <Common.hpp>
#include <deque>
#include <condition_variable>
#include <mutex>


namespace PS4::OS::Libs::Kernel {

struct Eflag {
    std::string name;
};

struct SceKernelEventFlagOptParam;
using SceKernelEventFlag = Eflag*;

s32 PS4_FUNC sceKernelCreateEventFlag(SceKernelEventFlag* ef, const char* name, u32 attr, u64 init_ptn, const SceKernelEventFlagOptParam* opt_param);
s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout);

};  // End namespace PS4::OS::Libs::Kernel