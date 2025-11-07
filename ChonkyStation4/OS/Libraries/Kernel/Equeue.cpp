#include "Equeue.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_equeue);

void EqueueData::trigger(u64 ident, u16 filter, u64 data) {
    for (auto& ev : events) {
        if (ev.ident == ident && ev.filter == filter) {
            // TODO: Actually signal that it was triggered
            ev.data = data;
        }
    }
}

s32 PS4_FUNC sceKernelCreateEqueue(SceKernelEqueue* eq, const char* name) {
    log("sceKernelCreateEqueue(eq=*%p, name=\"%s\")\n", eq, name);

    *eq = new EqueueData();
    (*eq)->name = name;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitEqueue(SceKernelEqueue eq, SceKernelEvent* ev, s32 n_evs, s32* n_out, u32* timeout) {
    log("sceKernelWaitEqueue(eq=*%p, ev=*%p, n_evs=%d, n_out=*%p, timeout=*%p) TODO\n", eq, ev, n_evs, n_out, timeout);

    // TODO: Stubbed to just return the first event
    // TODO: Check n_evs before writing
    *ev = eq->events[0];
    *n_out = 1;
    return SCE_OK;
}

};  // End namespace PS4::OS::Libs::Kernel