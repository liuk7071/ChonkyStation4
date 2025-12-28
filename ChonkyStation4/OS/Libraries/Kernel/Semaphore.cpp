#include "Semaphore.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_sema);

void Semaphore::signal(s32 count) {
    while (count--) {
        std_sema->release();
    }
}

void Semaphore::wait(s32 count) {
    auto lk = std::unique_lock<std::mutex>(mtx);

    while (count--) {
        std_sema->acquire();
    }
}

s32 PS4_FUNC sceKernelCreateSema(SceKernelSema* sem, const char* name, u32 attr, s32 init_count, s32 max_count, const SceKernelSemaOptParam* opt_param) {
    log("sceKernelCreateSema(sem=*%p, name=\"%s\", attr=0x%x, init_count=%d, max_count=%d, opt_param=*%p)\n", sem, name, attr, init_count, max_count, opt_param);

    *sem = new Semaphore(init_count, max_count);
    (*sem)->name = name;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelSignalSema(SceKernelSema sem, s32 count) {
    log("sceKernelSignalSema(sem=%p, count=%d)\n", sem, count);

    sem->signal(count);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitSema(SceKernelSema sem, s32 count, u32* timeout) {
    log("sceKernelWaitSema(sem=%p, count=%d, timeout=*%p)\n", sem, count, timeout);

    if (timeout && *timeout) {
        Helpers::panic("sceKernelWaitSema: timeout (todo)\n");
    }

    sem->wait(count);
    return SCE_OK;
}

};  // End namespace PS4::OS::Libs::Kernel