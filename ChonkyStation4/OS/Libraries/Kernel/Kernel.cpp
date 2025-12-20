#include "Kernel.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
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
#pragma intrinsic(__rdtsc)
#endif
#include <SDL.h>    // For performance counters


extern App g_app;

namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);
MAKE_LOG_FUNCTION(unimpl, unimplemented);

static u64 stack_chk_guard = 0x4452474B43415453;    // "STACKGRD"
//static u64 stack_chk_guard = 0;

void init(Module& module) {
    module.addSymbolExport("wtkt-teR1so", "pthread_attr_init", "libkernel", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("nsYoNRywwNg", "scePthreadAttrInit", "libkernel", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("Ucsu-OK+els", "pthread_attr_get_np", "libkernel", "libkernel", (void*)&kernel_pthread_attr_get_np);
    module.addSymbolExport("x1X76arYMxU", "scePthreadAttrGet", "libkernel", "libkernel", (void*)&kernel_pthread_attr_get_np);
    module.addSymbolExport("-wzZ7dvA7UU", "pthread_attr_getaffinity_np", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getaffinity_np);
    module.addSymbolExport("8+s5BzZjxSg", "scePthreadAttrGetaffinity", "libkernel", "libkernel", (void*)&scePthreadAttrGetaffinity);
    module.addSymbolExport("2Q0z6rnBrTE", "pthread_attr_setstacksize", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setstacksize);
    module.addSymbolExport("UTXzJbWhhTE", "scePthreadAttrSetstacksize", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setstacksize);
    module.addSymbolExport("E+tyo3lp5Lw", "pthread_attr_setdetachstate", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setdetachstate);
    module.addSymbolExport("-Wreprtu0Qs", "scePthreadAttrSetdetachstate", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setdetachstate);
    module.addSymbolExport("zHchY8ft5pk", "pthread_attr_destroy", "libkernel", "libkernel", (void*)&kernel_pthread_attr_destroy);
    module.addSymbolExport("62KCwEMmzcM", "scePthreadAttrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_attr_destroy);
    module.addSymbolExport("EotR8a3ASf4", "pthread_self", "libkernel", "libkernel", (void*)&kernel_pthread_self);
    module.addSymbolExport("aI+OeCz8xrQ", "scePthreadSelf", "libkernel", "libkernel", (void*)&kernel_pthread_self);

    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("9UK1vLZQft4", "scePthreadMutexLock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("K-jXhbt2gn4", "pthread_mutex_trylock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_trylock);
    module.addSymbolExport("upoVrzMHFeE", "scePthreadMutexTrylock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_trylock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("tn3VlD0hG60", "scePthreadMutexUnlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("F8bUHwAG284", "scePthreadMutexattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("smWEktiyyG0", "scePthreadMutexattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_destroy);
    module.addSymbolExport("iMp8QpE+XO4", "scePthreadMutexattrSettype", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("ttHNfU+qDBU", "pthread_mutex_init", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("ttHNfU+qDBU", "pthread_mutex_init", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("cmo1RIYva9o", "scePthreadMutexInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("2Of0f+3mhhE", "scePthreadMutexDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_destroy);

    module.addSymbolExport("mKoTx03HRWA", "pthread_condattr_init", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("m5-2bsNfv7s", "scePthreadCondattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("2Tb92quprl0", "scePthreadCondInit", "libkernel", "libkernel", (void*)&scePthreadCondInit);
    module.addSymbolExport("Op8TBGY5KHg", "pthread_cond_wait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("WKAXJ4XBPQ4", "scePthreadCondWait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("2MOy+rUfuhQ", "pthread_cond_signal", "libkernel", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("kDh-NfxgMtE", "scePthreadCondSignal", "libkernel", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("mkx2fVhNMsg", "pthread_cond_broadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("dJcuQVn6-Iw", "pthread_condattr_destroy", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    module.addSymbolExport("waPcxYiR3WA", "scePthreadCondattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    
    module.addSymbolExport("Z4QosVuAsA0", "pthread_once", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("14bOACANTBo", "scePthreadOnce", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("0-KXaS70xy4", "pthread_getspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("eoht7mQOCmo", "scePthreadGetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("WrOLvHU0yQM", "pthread_setspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("+BzXYkqYeLE", "scePthreadSetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("mqULNdimTn0", "pthread_key_create", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("geDaqgH9lTg", "scePthreadKeyCreate", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("6UgtwV+0zb4", "scePthreadCreate", "libkernel", "libkernel", (void*)&scePthreadCreate);
    module.addSymbolExport("+U1R4WtXvoc", "pthread_detach", "libkernel", "libkernel", (void*)&kernel_pthread_detach);
    module.addSymbolExport("4qGrR6eoP9Y", "scePthreadDetach", "libkernel", "libkernel", (void*)&kernel_pthread_detach);
    module.addSymbolExport("7Xl257M4VNI", "pthread_equal", "libkernel", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("3PtV6p3QNX4", "scePthreadEqual", "libkernel", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("B5GmVDKwpn0", "pthread_yield", "libkernel", "libkernel", (void*)&kernel_pthread_yield);
    module.addSymbolExport("T72hz6ffq08", "scePthreadYield", "libkernel", "libkernel", (void*)&kernel_pthread_yield);
    
    module.addSymbolExport("9BcDykPmo1I", "__error", "libkernel", "libkernel", (void*)&kernel_error);
    module.addSymbolExport("f7uOxY9mM1U", "__stack_chk_guard", "libkernel", "libkernel", (void*)&stack_chk_guard);
    module.addSymbolExport("vNe1w4diLCs", "__tls_get_addr", "libkernel", "libkernel", (void*)&__tls_get_addr);
    module.addSymbolExport("BPE9s9vQQXo", "mmap", "libkernel", "libkernel", (void*)&kernel_mmap);

    module.addSymbolExport("6c3rCVE-fTU", "_open", "libkernel", "libkernel", (void*)&kernel_open);
    module.addSymbolExport("1G3lF1Gg1k8", "sceKernelOpen", "libkernel", "libkernel", (void*)&sceKernelOpen);
    module.addSymbolExport("Oy6IpwgtYOk", "lseek", "libkernel", "libkernel", (void*)&kernel_lseek);
    module.addSymbolExport("oib76F-12fk", "sceKernelLseek", "libkernel", "libkernel", (void*)&sceKernelLseek);
    module.addSymbolExport("AqBioC2vF3I", "read", "libkernel", "libkernel", (void*)&kernel_read);
    module.addSymbolExport("Cg4srZ6TKbU", "sceKernelRead", "libkernel", "libkernel", (void*)&sceKernelRead);
    module.addSymbolExport("E6ao34wPw+U", "stat", "libkernel", "libkernel", (void*)&kernel_stat);
    module.addSymbolExport("eV9wAD2riIA", "sceKernelStat", "libkernel", "libkernel", (void*)&sceKernelStat);
    module.addSymbolExport("bY-PO6JhzhQ", "close", "libkernel", "libkernel", (void*)&kernel_close);
    module.addSymbolExport("UK2Tl2DWUns", "sceKernelClose", "libkernel", "libkernel", (void*)&sceKernelClose);
    module.addSymbolExport("YSHRBRLn2pI", "_writev", "libkernel", "libkernel", (void*)&kernel_writev);
    module.addSymbolExport("FxVZqBAA7ks", "_write", "libkernel", "libkernel", (void*)&kernel_write);
    
    module.addSymbolExport("HoLVWNanBBc", "getpid", "libkernel", "libkernel", (void*)&kernel_getpid);
    module.addSymbolExport("VkTAsrZDcJ0", "sigfillset", "libkernel", "libkernel", (void*)&sigfillset);
    
    module.addSymbolExport("7NwggrWJ5cA", "__sys_regmgr_call", "libkernel", "libkernel", (void*)&__sys_regmgr_call);

    module.addSymbolExport("yS8U2TGCe1A", "nanosleep", "libkernel", "libkernel", (void*)&kernel_nanosleep);
    module.addSymbolExport("yS8U2TGCe1A", "nanosleep", "libScePosix", "libkernel", (void*)&kernel_nanosleep);
    module.addSymbolExport("QcteRwbsnV0", "usleep", "libkernel", "libkernel", (void*)&sceKernelUsleep); // TODO: Technically should be a separate function, but the behavior should be the same
    module.addSymbolExport("1jfXLRVzisc", "sceKernelUsleep", "libkernel", "libkernel", (void*)&sceKernelUsleep);
    module.addSymbolExport("lLMT9vJAck0", "clock_gettime", "libkernel", "libkernel", (void*)&kernel_clock_gettime);
    module.addSymbolExport("lLMT9vJAck0", "clock_gettime", "libScePosix", "libkernel", (void*)&kernel_clock_gettime);
    module.addSymbolExport("QBi7HCK03hw", "sceKernelClockGettime", "libkernel", "libkernel", (void*)&sceKernelClockGettime);
    
    module.addSymbolExport("WslcK1FQcGI", "sceKernelIsNeoMode", "libkernel", "libkernel", (void*)&sceKernelIsNeoMode);
    module.addSymbolExport("959qrazPIrg", "sceKernelGetProcParam", "libkernel", "libkernel", (void*)&sceKernelGetProcParam);
    module.addSymbolExport("+g+UP8Pyfmo", "sceKernelGetProcessType", "libkernel", "libkernel", (void*)&sceKernelGetProcessType);
    module.addSymbolExport("p5EcQeEeJAE", "_sceKernelRtldSetApplicationHeapAPI", "libkernel", "libkernel", (void*)&_sceKernelRtldSetApplicationHeapAPI);
    module.addSymbolExport("1j3S3n-tTW4", "sceKernelGetTscFrequency", "libkernel", "libkernel", (void*)&sceKernelGetTscFrequency);

    module.addSymbolExport("D0OdFMjp46I", "sceKernelCreateEqueue", "libkernel", "libkernel", (void*)&sceKernelCreateEqueue);
    module.addSymbolExport("fzyMKs9kim0", "sceKernelWaitEqueue", "libkernel", "libkernel", (void*)&sceKernelWaitEqueue);
    
    module.addSymbolExport("B+vc2AO2Zrc", "sceKernelAllocateMainDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateMainDirectMemory);
    module.addSymbolExport("rTXw65xmLIA", "sceKernelAllocateDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateDirectMemory);
    module.addSymbolExport("L-Q3LEjIbgA", "sceKernelMapDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapDirectMemory);
    module.addSymbolExport("mL8NDH86iQI", "sceKernelMapNamedFlexibleMemory", "libkernel", "libkernel", (void*)&sceKernelMapNamedFlexibleMemory);
    module.addSymbolExport("cQke9UuBQOk", "sceKernelMunmap", "libkernel", "libkernel", (void*)&sceKernelMunmap);
    module.addSymbolExport("pO96TwzOm5E", "sceKernelGetDirectMemorySize", "libkernel", "libkernel", (void*)&sceKernelGetDirectMemorySize);
    
    module.addSymbolStub("1FGvU0i9saQ", "scePthreadMutexattrSetprotocol", "libkernel", "libkernel");
    module.addSymbolStub("WB66evu8bsU", "sceKernelGetCompiledSdkVersion", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("6xVpy0Fdq+I", "_sigprocmask", "libkernel", "libkernel");
    module.addSymbolStub("jh+8XiK4LeE", "sceKernelIsAddressSanitizerEnabled", "libkernel", "libkernel", false);
    module.addSymbolStub("bt3CTBKmGyI", "scePthreadSetaffinity", "libkernel", "libkernel");
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

s32* PS4_FUNC kernel_error() {
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

s32 PS4_FUNC kernel_nanosleep(SceKernelTimespec* rqtp, SceKernelTimespec* rmtp) {
    log("nanosleep(rqtp=*%p, rmtp=*%p)\n", rqtp, rmtp);

    const auto sec = std::chrono::seconds(rqtp->tv_sec);
    const auto nsec = std::chrono::nanoseconds(rqtp->tv_nsec);
    std::this_thread::sleep_for(sec + nsec);
    if (rmtp) {
        rmtp->tv_sec = 0;
        rmtp->tv_nsec = 0;
    }
    return 0;
}

s32 PS4_FUNC sceKernelUsleep(u32 us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
    return SCE_OK;
}

s32 PS4_FUNC kernel_clock_gettime(u32 clock_id, SceKernelTimespec* ts) {
    log("clock_gettime(clock_id=%d, ts=*%p)\n", clock_id, ts);

    switch (clock_id) {
    case SCE_KERNEL_CLOCK_MONOTONIC: {
        auto counter = SDL_GetPerformanceCounter();
        auto freq = SDL_GetPerformanceFrequency();
        ts->tv_sec = counter / freq;
        ts->tv_nsec = (counter % freq) * 1000000000ull / freq;
        break;
    }

    case SCE_KERNEL_CLOCK_SECOND: {
        auto now = std::chrono::system_clock::now();
        auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);

        ts->tv_sec = sec.time_since_epoch().count();
        ts->tv_nsec = ns.count();
        break;
    }
    
    default: Helpers::panic("clock_gettime: unhandled clock_id=%d\n", clock_id);
    }

    return 0;
}

s32 PS4_FUNC sceKernelClockGettime(u32 clock_id, SceKernelTimespec* ts) {
    const auto res = kernel_clock_gettime(clock_id, ts);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC sceKernelIsNeoMode() {
    log("sceKernelIsNeoMode()\n");
    return false;
}

void* PS4_FUNC sceKernelGetProcParam() {
    log("sceKernelGetProcParam()\n");
    return (void*)g_app.modules[0].proc_param_ptr;
}

s32 PS4_FUNC sceKernelGetProcessType() {
    // TODO
    log("sceKernelGetProcessType() TODO\n");
    return 0;
}

void PS4_FUNC _sceKernelRtldSetApplicationHeapAPI(void* api[]) {
    // TODO
    log("_sceKernelRtldSetApplicationHeapAPI()\n");
}

u64 PS4_FUNC sceKernelGetTscFrequency() {
    log("sceKernelGetTscFrequency()\n");

    u64 final_freq = 0;

#ifdef _WIN32
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER start;
    LARGE_INTEGER now;
    u64 first = __rdtsc();
    QueryPerformanceCounter(&start);
    for (;;) {
        u64 i = __rdtsc();
        QueryPerformanceCounter(&now);
        if (now.QuadPart - start.QuadPart >= freq.QuadPart) {
            final_freq = i - first;
            printf_s("Measured frequency: %lld\n", final_freq);
            break;
        }
    }
#else
    Helpers::panic("Unsupported platform\n");
#endif

    log("freq: %lld\n", final_freq);
    return final_freq;
}

s32 PS4_FUNC kernel_getpid() {
    log("getpid()\n");
    return 100;
}

s32 PS4_FUNC sigfillset() {
    // TODO
    unimpl("sigfillset() TODO\n");
    return SCE_OK;
}

s32 PS4_FUNC __sys_regmgr_call() {
    // TODO
    unimpl("__sys_regmgr_call() TODO\n");
    return SCE_OK;
}

// TODO: Properly implement the virtual memory map

s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr) {
    log("sceKernelAllocateMainDirectMemory(size=0x%016llx, align=0x%016llx, mem_type=%d, out_addr=*%p)\n", size, align, mem_type, out_addr);

    // TODO: For now we allocate memory directly in the map function
    //       Eventually I will need to handle the physical memory map properly...
    *out_addr = (void*)0x12345678;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr) {
    log("sceKernelAllocateMainDirectMemory(size=0x%016llx, align=0x%016llx, mem_type=%d, out_addr=*%p)\n", size, align, mem_type, out_addr);

    // TODO: For now we allocate memory directly in the map function
    //       Eventually I will need to handle the physical memory map properly...
    *out_addr = (void*)0x12345678;
    return SCE_OK;
}

//void* last_alloc_addr = (void*)0x10000000000;
void* last_alloc_addr = (void*)0x7ffffc000;

s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align) {
    log("sceKernelMapDirectMemory(addr=*%p, len=%lld, prot=%d, flags=%d, dmem_start=0x%016llx, align=0x%016llx)\n", addr, len, prot, flags, dmem_start, align);

    void* in_addr = *addr;
    if (in_addr) {
        Helpers::panic("TODO: sceKernelMapDirectMemory with non-zero input addr\n");
    }

    // TODO: prot, flags, verify align is a valid value (multiple of 16kb)
#ifdef _WIN32
    //*addr = VirtualAllocAlignedBelow(len, align, 0x10000000000);
    *addr = VirtualAllocAlignedBelow(len, align, (u64)last_alloc_addr);
    last_alloc_addr = *addr;
#else
    Helpers::panic("Unsupported platform\n");
#endif

    // Clear allocated memory
    std::memset(*addr, 0, len);

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name) {
    log("sceKernelMapNamedFlexibleMemory(addr=*%p, len=%lld, prot=%d, flags=%d, name=\"%s\")\n", addr, len, prot, flags, name);

    void* in_addr = *addr;
    if (in_addr) {
        Helpers::panic("TODO: sceKernelMapNamedFlexibleMemory with non-zero input addr\n");
    }

    // TODO: prot, flags
#ifdef _WIN32
    //*addr = VirtualAllocAlignedBelow(len, 1, 0x10000000000);
    *addr = VirtualAllocAlignedBelow(len, 1, (u64)last_alloc_addr);
    last_alloc_addr = *addr;
#else
    Helpers::panic("Unsupported platform\n");
#endif

    // Clear allocated memory
    std::memset(*addr, 0, len);

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMunmap(void* addr, size_t len) {
    log("sceKernelMunmap(addr=%p, len=%lld)\n", addr, len);

#ifdef _WIN32
    VirtualFree(addr, len, MEM_RELEASE);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetDirectMemorySize() {
    log("sceKernelGetDirectMemorySize()\n");
    return 5_GB - 512_MB;   // total size - flexible mem size
}

}