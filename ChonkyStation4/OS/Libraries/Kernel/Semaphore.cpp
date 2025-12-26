#include "Semaphore.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_sema);

s32 PS4_FUNC sceKernelCreateSema(SceKernelSema* sem, const char* name, u32 attr, s32 init_count, s32 max_count, const SceKernelSemaOptParam* opt_param) {
    log("sceKernelCreateSema(sem=*%p, name=\"%s\", attr=0x%x, init_count=%d, max_count=%d, opt_param=*%p)\n", sem, name, attr, init_count, max_count, opt_param);

    *sem = new Semaphore(init_count, max_count);
    (*sem)->name = name;
    return SCE_OK;
}

};  // End namespace PS4::OS::Libs::Kernel