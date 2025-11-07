#pragma once

#include <Common.hpp>
#include <deque>


namespace PS4::OS::Libs::Kernel {

struct SceKernelEvent {
	u64 ident;
	u16 filter;
	u16 flags;
	u32 fflags;
	u64 data;
    void* udata;
};

struct EqueueData {
    std::string name;
    std::vector<SceKernelEvent> events;
    
    void registerEvent(SceKernelEvent ev) {
        // TODO: Verify ev.ident is unique
        events.push_back(ev);
    }

    void trigger(u64 ident, u16 filter, u64 data);
};

using SceKernelEqueue = EqueueData*;

s32 PS4_FUNC sceKernelCreateEqueue(SceKernelEqueue* eq, const char* name);
s32 PS4_FUNC sceKernelWaitEqueue(SceKernelEqueue eq, SceKernelEvent* ev, s32 n_evs, s32* n_out, u32* timeout);

};  // End namespace PS4::OS::Libs::Kernel