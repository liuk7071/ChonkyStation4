#pragma once

#include <Common.hpp>
#include <deque>
#include <condition_variable>
#include <mutex>


namespace PS4::OS::Libs::Kernel {

static constexpr s32 SCE_KERNEL_ERROR_ETIMEDOUT = 0x8002003c;

struct SceKernelEvent {
	u64 ident;
	u16 filter;
	u16 flags;
	u32 fflags;
	u64 data;
    void* udata;
};

struct Equeue {
    std::string name;
    std::vector<std::pair<bool, SceKernelEvent>> events;    // bool = event arrived or not
    std::condition_variable cv;
    std::mutex cv_m;

    
    void registerEvent(SceKernelEvent ev) {
        // TODO: Verify ev.ident is unique
        events.push_back({ false, ev });
    }

    void trigger(u64 ident, u16 filter, u64 data);
    std::pair<bool, std::vector<SceKernelEvent>> wait(u32 timeout); // bool = whether or not we timed out
};

using SceKernelEqueue = Equeue*;

s32 PS4_FUNC sceKernelCreateEqueue(SceKernelEqueue* eq, const char* name);
s32 PS4_FUNC sceKernelWaitEqueue(SceKernelEqueue eq, SceKernelEvent* ev, s32 n_evs, s32* n_out, u32* timeout);

};  // End namespace PS4::OS::Libs::Kernel