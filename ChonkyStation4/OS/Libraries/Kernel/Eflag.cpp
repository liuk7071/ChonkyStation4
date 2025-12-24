#include "Eflag.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_equeue);

s32 PS4_FUNC sceKernelWaitEventFlag(SceKernelEventFlag ef, u64 bitptn, u32 wait_mode, u64* result, u32* timeout) {
    log("sceKernelWaitEventFlag(ef=%p, bitptn=0x%016llx, wait_mode=0x%x, result=*%p, timeout=*%p)\n", ef, bitptn, wait_mode, result, timeout);

    // TODO
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::Kernel