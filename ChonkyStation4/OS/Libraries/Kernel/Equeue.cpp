#include "Equeue.hpp"
#include <Logger.hpp>
#include <thread>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_equeue);

// TODO: level triggered events

void Equeue::trigger(u64 ident, u16 filter, u64 data) {
    bool has_triggered_event = false;
    for (auto& ev : events) {
        if (ev.second.ident == ident && ev.second.filter == filter) {
            ev.first = true;    // Event arrived
            has_triggered_event = true;
            ev.second.data = data;
        }
    }

    // Notify equeue condition variable
    std::unique_lock lk(cv_m);
    cv.notify_one();
}

std::pair<bool, std::vector<SceKernelEvent>> Equeue::wait(u32 timeout) {
    auto poll_events = [&]() -> std::vector<SceKernelEvent> {
        std::vector<SceKernelEvent> evs;
        for (auto& ev : events) {
            // Check if the event arrived and add it to the list of arrived events
            if (ev.first) {
                ev.first = false;   // Reset arrived state
                evs.push_back(ev.second);
            }
        }
        return evs;
    };

    std::vector<SceKernelEvent> evs;

    // Poll events once to see if there already were arrived events
    evs = poll_events();
    if (!evs.empty()) return { false, evs };    // false means we did not timeout

    // Waiting for events is implemented via a condition variable
    if (!timeout) {
        // Wait for events without timeout 
        std::unique_lock lk(cv_m);
        cv.wait(lk);
    }
    else {
        // Wait for events with timeout 
        std::unique_lock lk(cv_m);
        auto res = cv.wait_for(lk, std::chrono::microseconds(timeout));
        // Did we timeout?
        if (res == std::cv_status::timeout) return { true, {} };
    }

    // Return events
    evs = poll_events();
    return { false, evs };
}

void EventSource::init(u64 ident, u16 filter) {
    Helpers::debugAssert(!initialized, "Tried to initialize event source twice\n");
    initialized = true;
    this->ident = ident;
    this->filter = filter;
}

void EventSource::addToEventQueue(Equeue* eq, void* udata) {
    eq->registerEvent({
        .ident = ident,
        .filter = filter,
        .flags = 0,     // TODO
        .fflags = 0,    // TODO
        .data = 0,      
        .udata = udata,
    });
    eqs.push_back(eq);
}

void EventSource::trigger(u64 data) {
    for (auto& eq : eqs) {
        eq->trigger(ident, filter, data);
    }
}

s32 PS4_FUNC sceKernelCreateEqueue(SceKernelEqueue* eq, const char* name) {
    log("sceKernelCreateEqueue(eq=*%p, name=\"%s\")\n", eq, name);

    *eq = new Equeue();
    (*eq)->name = name;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitEqueue(SceKernelEqueue eq, SceKernelEvent* ev, s32 n_evs, s32* n_out, u32* timeout) {
    log("sceKernelWaitEqueue(eq=*%p, ev=*%p, n_evs=%d, n_out=*%p, timeout=*%p)\n", eq, ev, n_evs, n_out, timeout);

    auto [timed_out, events] = eq->wait(timeout ? *timeout : 0);
    if (timed_out) return SCE_KERNEL_ERROR_ETIMEDOUT;

    // Return events
    int written_evs = 0;
    for (int i = 0; i < events.size(); i++) {
        if (i >= n_evs) break;

        ev[i] = events[i];
        written_evs++;
    }

    *n_out = written_evs;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelAddUserEvent(SceKernelEqueue eq, s32 id) {
    log("sceKernelAddUserEvent(eq=%p, id=%d)\n", eq, id);

    eq->registerEvent({
        .ident = (u64)id,
        .filter = 0,
        .flags = 0,     
        .fflags = 0,    
        .data = 0,
        .udata = 0,
    });
    return SCE_OK;
}

};  // End namespace PS4::OS::Libs::Kernel