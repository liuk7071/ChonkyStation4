#include "Eflag.hpp"
#include <Logger.hpp>
#ifdef _MSC_VER
#include <intrin.h>
#define RETURN_ADDRESS() _ReturnAddress()
#else
#define RETURN_ADDRESS() _builtin_return_address(0)
#endif


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_eflag);

s32 PS4_FUNC sceKernelCreateEventFlag(SceKernelEventFlag* ef, const char* name, u32 attr, u64 init_ptn, const SceKernelEventFlagOptParam* opt_param) {
    log("sceKernelCreateEventFlag(ef=*%p, name=\"%s\", attr=0x%x, init_ptn=0x%llx, opt_param=*%p)\n", ef, name, attr, init_ptn, opt_param);
    log("ret=%p\n", RETURN_ADDRESS());
    *ef = new Eflag();
    (*ef)->name = name;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout) {
    log("sceKernelWaitEventFlag(ef=%p, bitptn=0x%016llx, wait_mode=0x%x, result=*%p, timeout=*%p)\n", ef, bitptn, wait_mode, result, timeout);

    // TODO
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::Kernel