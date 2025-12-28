#include "Eflag.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
#ifdef _MSC_VER
#include <intrin.h>
#define RETURN_ADDRESS() _ReturnAddress()
#else
#define RETURN_ADDRESS() _builtin_return_address(0)
#endif


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_eflag);

static constexpr u32 SCE_KERNEL_EVF_WAITMODE_AND        = 0x01;
static constexpr u32 SCE_KERNEL_EVF_WAITMODE_OR         = 0x02;
static constexpr u32 SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL  = 0x10;
static constexpr u32 SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT  = 0x20;
static constexpr u32 SCE_KERNEL_EVF_ATTR_SINGLE         = 0x10;
static constexpr u32 SCE_KERNEL_EVF_ATTR_MULTI          = 0x20;

bool Eflag::checkCond(const Eflag::Waiter& waiter) {
    if (waiter.wait_mode & SCE_KERNEL_EVF_WAITMODE_AND)
        return (bitptn & waiter.bitptn) == waiter.bitptn;
    else if (waiter.wait_mode & SCE_KERNEL_EVF_WAITMODE_OR)
        return bitptn & waiter.bitptn;
    Helpers::panic("Eflag::wait: invalid wait_mode");
}

// Returns true if the waiter woke up
bool Eflag::maybeWakeup(Eflag::Waiter& waiter) {
    if (checkCond(waiter)) {
        waiter.result = this->bitptn;
        waiter.check_done = true;
        waiter.cv.notify_one();
        // Check if we need to clear the flag
        if (waiter.wait_mode & SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL)       this->bitptn = 0;
        else if (waiter.wait_mode & SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT)  this->bitptn &= ~waiter.bitptn;
        return true;
    }
    return false;
}

void Eflag::set(u64 bitptn) {
    auto lk = std::unique_lock<std::mutex>(mtx);
    this->bitptn |= bitptn;

    // Traverse wait list and wake up if necessary
    for (auto it = waiters.begin(); it != waiters.end(); ) {
        Waiter& waiter = **it;
        if (maybeWakeup(waiter))    it = waiters.erase(it);
        else                        it++;
    }
}

void Eflag::clear(u64 bitptn) {
    auto lk = std::unique_lock<std::mutex>(mtx);
    this->bitptn = this->bitptn & bitptn;
    // Clearing a flag can't wake up any threads so there is nothing else to do
}

bool Eflag::wait(u64 bitptn, u32 wait_mode, u64& result) {
    auto lk = std::unique_lock<std::mutex>(mtx);
    
    if (!is_multi) {
        if (waiters.size()) {
            result = SCE_KERNEL_ERROR_EPERM;
            return false;
        }
    }

    auto waiter = std::make_shared<Waiter>();
    waiter->bitptn = bitptn;
    waiter->wait_mode = wait_mode;

    // Do an early check and avoid sleeping if the condition is already met
    if (!maybeWakeup(*waiter)) {
        // Only wait if the condition wasn't met or the queue was not empty
        waiters.push_back(waiter);
        waiter->cv.wait(lk, [&]() { return waiter->check_done; });
    }

    result = waiter->result;
    return true;
}

bool Eflag::poll(u64 bitptn, u32 wait_mode, u64& result) {
    auto lk = std::unique_lock<std::mutex>(mtx);

    if (!is_multi) {
        if (waiters.size()) {
            result = SCE_KERNEL_ERROR_EPERM;
            return false;
        }
    }

    auto waiter = std::make_shared<Waiter>();
    waiter->bitptn = bitptn;
    waiter->wait_mode = wait_mode;

    if (!maybeWakeup(*waiter)) {
        // Return error
        result = SCE_KERNEL_ERROR_EBUSY;
        return false;
    }

    result = waiter->result;
    return true;
}

s32 PS4_FUNC sceKernelCreateEventFlag(SceKernelEventFlag* ef, const char* name, u32 attr, u64 init_ptn, const SceKernelEventFlagOptParam* opt_param) {
    log("sceKernelCreateEventFlag(ef=*%p, name=\"%s\", attr=0x%x, init_ptn=0x%llx, opt_param=*%p)\n", ef, name, attr, init_ptn, opt_param);

    *ef = new Eflag();
    (*ef)->name = name;
    (*ef)->bitptn = init_ptn;
    (*ef)->is_multi = attr & SCE_KERNEL_EVF_ATTR_MULTI;
    log("handle: %p\n", *ef);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelSetEventFlag(SceKernelEventFlag ef, u64 bitptn) {
    log("sceKernelSetEventFlag(ef=%p, bitptn=0x%016llx)\n", ef, bitptn);

    ef->set(bitptn);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelClearEventFlag(SceKernelEventFlag ef, u64 bitptn) {
    log("sceKernelClearEventFlag(ef=%p, bitptn=0x%016llx)\n", ef, bitptn);

    ef->clear(bitptn);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout) {
    log("sceKernelWaitEventFlag(ef=%p, bitptn=0x%016llx, wait_mode=0x%x, result=*%p, timeout=*%p)\n", ef, bitptn, wait_mode, result, timeout);

    // TODO: Error checks
    if (timeout && *timeout) {
        Helpers::panic("TODO: event flag timeout\n");
    }

    u64 ret;
    if (ef->wait(bitptn, wait_mode, ret)) {
        if (result) *result = ret;
        return SCE_OK;
    }
    
    // An error happened
    return ret;
}

s32 PS4_FUNC sceKernelPollEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result) {
    log("sceKernelPollEventFlag(ef=%p, bitptn=0x%016llx, wait_mode=0x%x, result=*%p)\n", ef, bitptn, wait_mode, result);

    u64 ret;
    if (ef->poll(bitptn, wait_mode, ret)) {
        if (result) *result = ret;
        return SCE_OK;
    }

    // An error happened
    return ret;
}

}   // End namespace PS4::OS::Libs::Kernel