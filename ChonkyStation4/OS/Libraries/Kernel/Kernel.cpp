#include "Kernel.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/kernel/pthread/pthread.hpp>
#include <OS/Libraries/kernel/pthread/mutex.hpp>
#include <OS/Libraries/kernel/pthread/cond.hpp>
#include <chrono>
#include <thread>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

void init(Module& module) {
    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("mkx2fVhNMsg", "pthread_cond_broadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("9BcDykPmo1I", "__error", "libkernel", "libkernel", (void*)&___error);
    module.addSymbolExport("BPE9s9vQQXo", "mmap", "libkernel", "libkernel", (void*)&kernel_mmap);
    module.addSymbolExport("Z4QosVuAsA0", "pthread_once", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("0-KXaS70xy4", "pthread_getspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("WrOLvHU0yQM", "pthread_setspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("mqULNdimTn0", "pthread_key_create", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("YSHRBRLn2pI", "_writev", "libkernel", "libkernel", (void*)&kernel_writev);
    module.addSymbolExport("1jfXLRVzisc", "sceKernelUsleep", "libkernel", "libkernel", (void*)&sceKernelUsleep);
}

static thread_local s32 posix_errno = 0;

s32* PS4_FUNC ___error() {
    return &posix_errno;
}

void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs) {
    log("mmap(addr=%p, len=0x%llx, prot=0x%x, flags=0x%x, fd=%d, offs=0x%llx)\n", addr, len, prot, flags, fd, offs);
    // TODO: This is stubbed as malloc for now
    return std::malloc(len);
}

size_t PS4_FUNC kernel_writev(s32 fd, KernelIovec* iov, int iovcnt) {
    log("_writev(fd=%d, iov=%p, iovcnt=%d)\n", fd, iov, iovcnt);
    
    size_t written = 0;
    for (int i = 0; i < iovcnt; i++) {
        char* ptr = nullptr;
        for (ptr = (char*)iov[i].iov_base; ptr < (char*)iov[i].iov_base + iov[i].iov_len; ptr++)
            std::putc(*ptr, stdout);

        std::putc('\n', stdout);
        written += iov[i].iov_len;
    }
    
    return written;
}

s32 PS4_FUNC sceKernelUsleep(u32 us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
    return 0;
}

}