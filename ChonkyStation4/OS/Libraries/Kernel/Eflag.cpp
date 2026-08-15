#include "Eflag.hpp"
#include <Logger.hpp>
#include <GCN/GCN.hpp>
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

s32 PS4_FUNC sceKernelOpenEventFlag(SceKernelEventFlag* ef, const char* name) {
    log("sceKernelOpenEventFlag(ef=*%p, name=\"%s\")\n", ef, name);

    *ef = new Eflag();
    (*ef)->name = name;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelSetEventFlag(SceKernelEventFlag ef, u64 bitptn) {
    log("sceKernelSetEventFlag(ef=%p, bitptn=0x%016llx)\n", ef, bitptn);

    ef->set(bitptn);

    //if (ef->name == "SceShellUIBootManager") {
    //    printf("set boot manager: %llx\n", ef->bitptn);
    //}
    return SCE_OK;
}

s32 PS4_FUNC sceKernelClearEventFlag(SceKernelEventFlag ef, u64 bitptn) {
    log("sceKernelClearEventFlag(ef=%p, bitptn=0x%016llx)\n", ef, bitptn);

    ef->clear(bitptn);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout) {
    log("sceKernelWaitEventFlag(ef=%p, bitptn=0x%016llx, wait_mode=0x%x, result=*%p, timeout=*%p)\n", ef, bitptn, wait_mode, result, timeout);
    
    if (ef->name == "SceBootStatusFlags") return SCE_OK;
    if (ef->name == "SceCompositorEventflag") return SCE_OK;
    if (ef->name == "SceCompositorResetStatusEVF") return SCE_OK;
    if (ef->name == "SceCompositorSysSusEq") return SCE_OK;

    // TODO: Error checks
    if (timeout && *timeout) {
        //Helpers::panic("TODO: event flag timeout\n");
        printf("TODO: event flag timeout\n");
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

    if (ef->name == "SceShellUIBootManager") {
        //printf("boot manager: %llx\n", ef->bitptn);
        //printf("boot manager polled\n");
        ef->bitptn |= 0x00000000800000;    // BackgroundInitFinishedBeforeBGLayer
        ef->bitptn |= 0x00000001000000;    // BackgroundInitFinishedBeforeBasePlugin
        ef->bitptn |= 0x00001000000000;    // MainOnStandbyFinished
        ef->bitptn |= 0x04000000000000;    // RegMgrInitOKCompleted
        //if (GCN::global_flip_counter > 10) {
            //ef->bitptn |= 0x01000000000000;    // SelectResolutionCompleted
            //ef->bitptn |= 0x00000000100000;    // DBRecoveryRequestChecked
            //ef->bitptn |= 0x00000000000100;    // InitialSetupCompleted
            //ef->bitptn |= 0x00000000020000;    // CrashReportPluginCompleted
            //ef->bitptn |= 0x00000000001000;    // PowerOffWarningFinished
            //ef->bitptn |= 0x00008000000000;    // CreateKratosUserCompleted
            //ef->bitptn |= 0x10000000000000;    // NotifyDBInitializedStarted
            //ef->bitptn |= 0x00020000000000;    // AutoStandbyAnnouncementFinished
            //ef->bitptn |= 0x00000200000000;    // SystemUpdateCompleted
            //ef->bitptn |= 0x00100000000000;    // SystemPasscodeInputCompleted
        //}
        //ef->bitptn |= 0x00000040000000;    // HealthWarningStarted
        //ef->bitptn |= 0x20000000000000;    // NotifyDBInitializedFinished
        //ef->bitptn |= 0x80000000000000;    // PlatformPrivacyCompleted
        //ef->bitptn |= 0x00000002000000;    // BackgroundInitFinishedBeforeFarsightUI
        //ef->bitptn |= 0x00002000000000;    // BackgroundAutoLoginFinished
        //ef->bitptn |= 0x00000000000400;    // BasePluginLoadFinished
    }

    if (ef->name == "SceCompositorEventflag") return SCE_OK;
    if (ef->name == "SceCompositorResetStatusEVF") return SCE_OK;
    if (ef->name == "SceCompositorSysSusEq") return SCE_OK;

    u64 ret;
    if (ef->poll(bitptn, wait_mode, ret)) {
        if (result) *result = ret;
        return SCE_OK;
    }

    // An error happened
    return ret;
}

}   // End namespace PS4::OS::Libs::Kernel