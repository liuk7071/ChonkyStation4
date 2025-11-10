#include "Kernel.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/Kernel/pthread/pthread.hpp>
#include <OS/Libraries/Kernel/pthread/mutex.hpp>
#include <OS/Libraries/Kernel/pthread/cond.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
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
    module.addSymbolExport("FxVZqBAA7ks", "_write", "libkernel", "libkernel", (void*)&kernel_write);
    module.addSymbolExport("1jfXLRVzisc", "sceKernelUsleep", "libkernel", "libkernel", (void*)&sceKernelUsleep);
    module.addSymbolExport("WslcK1FQcGI", "sceKernelIsNeoMode", "libkernel", "libkernel", (void*)&sceKernelIsNeoMode);

    module.addSymbolExport("D0OdFMjp46I", "sceKernelCreateEqueue", "libkernel", "libkernel", (void*)&sceKernelCreateEqueue);
    module.addSymbolExport("fzyMKs9kim0", "sceKernelWaitEqueue", "libkernel", "libkernel", (void*)&sceKernelWaitEqueue);
    
    module.addSymbolExport("B+vc2AO2Zrc", "sceKernelAllocateMainDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateMainDirectMemory);
    module.addSymbolExport("L-Q3LEjIbgA", "sceKernelMapDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapDirectMemory);
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
    //log("_writev(fd=%d, iov=%p, iovcnt=%d)\n", fd, iov, iovcnt);

    if (fd != 0 && fd != 1) {
        Helpers::panic("_writev: fd is not stdin or stdout (TODO)\n");
    }
    
    size_t written = 0;
    for (int i = 0; i < iovcnt; i++) {
        char* ptr = nullptr;
        for (ptr = (char*)iov[i].iov_base; ptr < (char*)iov[i].iov_base + iov[i].iov_len; ptr++)
            std::putc(*ptr, stdout);

        //std::putc('\n', stdout);
        written += iov[i].iov_len;
    }
    
    return written;
}

size_t PS4_FUNC kernel_write(s32 fd, const void* buf, size_t size) {
    //log("_write(fd=%d, buf=%p, size=%d)\n", fd, buf, size);

    if (fd != 0 && fd != 1) {
        Helpers::panic("_write: fd is not stdin or stdout (TODO)\n");
    }

    for (char* ptr = (char*)buf; ptr < (char*)buf + size; ptr++)
        std::putc(*ptr, stdout);
    return size;
}

s32 PS4_FUNC sceKernelUsleep(u32 us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
    return SCE_OK;
}

s32 PS4_FUNC sceKernelIsNeoMode() {
    log("sceKernelIsNeoMode()\n");
    return false;
}

s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr) {
    log("sceKernelAllocateMainDirectMemory(size=0x%016llx, align=0x%016llx, mem_type=%d, out_addr=*%p)\n", size, align, mem_type, out_addr);

    // TODO: For now we allocate memory directly in the map function
    //       Eventually I will need to handle the physical memory map properly...
    *out_addr = (void*)0x12345678;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align) {
    log("sceKernelMapDirectMemory(addr=*%p, len=%lld, prot=%d, flags=%d, dmem_start=0x%016llx, align=%lld)\n", addr, len, prot, flags, dmem_start, align);

    void* in_addr = *addr;
    if (in_addr) {
        Helpers::panic("TODO: sceKernelMapDirectMemory with non-zero input addr\n");
    }

    // TODO: prot, flags, verify align is a valid value (multiple of 16kb)
#ifdef _WIN32
    *addr = _aligned_malloc(len, align);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

}