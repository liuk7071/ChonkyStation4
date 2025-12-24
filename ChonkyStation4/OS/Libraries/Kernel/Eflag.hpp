#pragma once

#include <Common.hpp>
#include <deque>
#include <condition_variable>
#include <mutex>


namespace PS4::OS::Libs::Kernel {

struct Eflag {
    std::string name;
};

using SceKernelEventFlag = Eflag*;

s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout);

};  // End namespace PS4::OS::Libs::Kernel