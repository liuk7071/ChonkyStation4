#include "Kernel.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <Loaders/App.hpp>
#include <OS/Thread.hpp>
#include <OS/Libraries/Kernel/pthread/pthread.hpp>
#include <OS/Libraries/Kernel/pthread/mutex.hpp>
#include <OS/Libraries/Kernel/pthread/cond.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
#include <OS/Libraries/Kernel/Filesystem.hpp>
#include <chrono>
#include <thread>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif


extern App g_app;

namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

void init(Module& module) {
    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("9UK1vLZQft4", "scePthreadMutexLock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("tn3VlD0hG60", "scePthreadMutexUnlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("F8bUHwAG284", "scePthreadMutexattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("smWEktiyyG0", "scePthreadMutexattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_destroy);
    module.addSymbolExport("iMp8QpE+XO4", "scePthreadMutexattrSettype", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("cmo1RIYva9o", "scePthreadMutexInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("mkx2fVhNMsg", "pthread_cond_broadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("Z4QosVuAsA0", "pthread_once", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("14bOACANTBo", "scePthreadOnce", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("0-KXaS70xy4", "pthread_getspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("eoht7mQOCmo", "scePthreadGetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("WrOLvHU0yQM", "pthread_setspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("+BzXYkqYeLE", "scePthreadSetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("mqULNdimTn0", "pthread_key_create", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("geDaqgH9lTg", "scePthreadKeyCreate", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    
    module.addSymbolExport("9BcDykPmo1I", "__error", "libkernel", "libkernel", (void*)&___error);
    module.addSymbolExport("vNe1w4diLCs", "__tls_get_addr", "libkernel", "libkernel", (void*)&__tls_get_addr);
    module.addSymbolExport("BPE9s9vQQXo", "mmap", "libkernel", "libkernel", (void*)&kernel_mmap);

    module.addSymbolExport("6c3rCVE-fTU", "_open", "libkernel", "libkernel", (void*)&kernel_open);
    module.addSymbolExport("YSHRBRLn2pI", "_writev", "libkernel", "libkernel", (void*)&kernel_writev);
    module.addSymbolExport("FxVZqBAA7ks", "_write", "libkernel", "libkernel", (void*)&kernel_write);
    
    module.addSymbolExport("VkTAsrZDcJ0", "sigfillset", "libkernel", "libkernel", (void*)&sigfillset);

    module.addSymbolExport("1jfXLRVzisc", "sceKernelUsleep", "libkernel", "libkernel", (void*)&sceKernelUsleep);
    
    module.addSymbolExport("WslcK1FQcGI", "sceKernelIsNeoMode", "libkernel", "libkernel", (void*)&sceKernelIsNeoMode);
    module.addSymbolExport("959qrazPIrg", "sceKernelGetProcParam", "libkernel", "libkernel", (void*)&sceKernelGetProcParam);
    module.addSymbolExport("p5EcQeEeJAE", "_sceKernelRtldSetApplicationHeapAPI", "libkernel", "libkernel", (void*)&_sceKernelRtldSetApplicationHeapAPI);

    module.addSymbolExport("D0OdFMjp46I", "sceKernelCreateEqueue", "libkernel", "libkernel", (void*)&sceKernelCreateEqueue);
    module.addSymbolExport("fzyMKs9kim0", "sceKernelWaitEqueue", "libkernel", "libkernel", (void*)&sceKernelWaitEqueue);
    
    module.addSymbolExport("B+vc2AO2Zrc", "sceKernelAllocateMainDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateMainDirectMemory);
    module.addSymbolExport("L-Q3LEjIbgA", "sceKernelMapDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapDirectMemory);
}

static thread_local s32 posix_errno = 0;

#ifdef _WIN32
static void* VirtualAllocAlignedBelow(size_t size, size_t alignment, uptr maxAddress) {
    if ((alignment & (alignment - 1)) != 0) return NULL;

    SYSTEM_INFO si; GetSystemInfo(&si);
    size_t gran = si.dwAllocationGranularity;
    size_t reserveSize = size + alignment;

    MEMORY_BASIC_INFORMATION mbi;
    uptr addr = maxAddress ? maxAddress - 1 : 0;

    while (addr > 0) {
        if (!VirtualQuery((void*)addr, &mbi, sizeof(mbi))) break;

        if (mbi.State == MEM_FREE) {
            uptr base = (uptr)mbi.BaseAddress;
            uptr end  = base + mbi.RegionSize;
            if (end > maxAddress) end = maxAddress;

            if (base + reserveSize <= end) {
                uptr tryBase = end - reserveSize;

                void* reserve = VirtualAlloc((void*)tryBase, reserveSize, MEM_RESERVE, PAGE_READWRITE);
                if (reserve) {
                    uptr rbase = (uptr)reserve;
                    uptr aligned = (rbase + (alignment - 1)) & ~(alignment - 1);

                    void* commit = VirtualAlloc((void*)aligned, size, MEM_COMMIT, PAGE_READWRITE);
                    if (!commit) { VirtualFree(reserve, 0, MEM_RELEASE); return NULL; }

                    if (aligned > rbase)
                        VirtualFree((void*)rbase, aligned - rbase, MEM_RELEASE);

                    uptr endAligned = aligned + size;
                    uptr endReserve = rbase + reserveSize;
                    if (endAligned < endReserve)
                        VirtualFree((void*)endAligned, endReserve - endAligned, MEM_RELEASE);

                    return commit;
                }
            }
        }

        if ((uptr)mbi.BaseAddress <= gran) break;
        addr = (uptr)mbi.BaseAddress - gran;
    }

    return NULL;
}
#endif

s32* PS4_FUNC ___error() {
    return &posix_errno;
}

void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx) {
    return (void*)((u64)Thread::getTLSPtr(tls_idx->modid) + tls_idx->offset);
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

        written += iov[i].iov_len;
    }
    
    return written;
}

size_t PS4_FUNC kernel_write(s32 fd, const void* buf, size_t size) {
    //log("_write(fd=%d, buf=%p, size=%d)\n", fd, buf, size);

    if (fd != 0 && fd != 1 && fd != 2) {
        Helpers::panic("_write: fd is not stdin or stdout or stderr (TODO)\n");
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

void* PS4_FUNC sceKernelGetProcParam() {
    log("sceKernelGetProcParam()\n");
    return (void*)g_app.modules[0].proc_param_ptr;
}

void PS4_FUNC _sceKernelRtldSetApplicationHeapAPI(void* api[]) {
    // TODO
    log("_sceKernelRtldSetApplicationHeapAPI()\n");
}

s32 sigfillset() {
    // TODO
    log("sigfillset() TODO\n");
    return SCE_OK;
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
    *addr = VirtualAllocAlignedBelow(len, align, 0x10000000000);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

}