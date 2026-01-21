#include "Kernel.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
#include <Loaders/Module.hpp>
#include <Loaders/App.hpp>
#include <OS/Thread.hpp>
#include <OS/Libraries/Kernel/pthread/pthread.hpp>
#include <OS/Libraries/Kernel/pthread/mutex.hpp>
#include <OS/Libraries/Kernel/pthread/cond.hpp>
#include <OS/Libraries/Kernel/pthread/rwlock.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
#include <OS/Libraries/Kernel/Eflag.hpp>
#include <OS/Libraries/Kernel/Semaphore.hpp>
#include <OS/Libraries/Kernel/Filesystem.hpp>
#include <chrono>
#include <thread>
#include <mutex>
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
    module.addSymbolExport("wtkt-teR1so", "pthread_attr_init", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("nsYoNRywwNg", "scePthreadAttrInit", "libkernel", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("Ucsu-OK+els", "pthread_attr_get_np", "libkernel", "libkernel", (void*)&kernel_pthread_attr_get_np);
    module.addSymbolExport("x1X76arYMxU", "scePthreadAttrGet", "libkernel", "libkernel", (void*)&kernel_pthread_attr_get_np);
    module.addSymbolExport("-wzZ7dvA7UU", "pthread_attr_getaffinity_np", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getaffinity_np);
    module.addSymbolExport("8+s5BzZjxSg", "scePthreadAttrGetaffinity", "libkernel", "libkernel", (void*)&scePthreadAttrGetaffinity);
    module.addSymbolExport("2Q0z6rnBrTE", "pthread_attr_setstacksize", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setstacksize);
    module.addSymbolExport("2Q0z6rnBrTE", "pthread_attr_setstacksize", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_setstacksize);
    module.addSymbolExport("UTXzJbWhhTE", "scePthreadAttrSetstacksize", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setstacksize);
    module.addSymbolExport("E+tyo3lp5Lw", "pthread_attr_setdetachstate", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setdetachstate);
    module.addSymbolExport("E+tyo3lp5Lw", "pthread_attr_setdetachstate", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_setdetachstate);
    module.addSymbolExport("-Wreprtu0Qs", "scePthreadAttrSetdetachstate", "libkernel", "libkernel", (void*)&kernel_pthread_attr_setdetachstate);
    module.addSymbolExport("zHchY8ft5pk", "pthread_attr_destroy", "libkernel", "libkernel", (void*)&kernel_pthread_attr_destroy);
    module.addSymbolExport("zHchY8ft5pk", "pthread_attr_destroy", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_destroy);
    module.addSymbolExport("62KCwEMmzcM", "scePthreadAttrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_attr_destroy);
    module.addSymbolExport("EotR8a3ASf4", "pthread_self", "libkernel", "libkernel", (void*)&kernel_pthread_self);
    module.addSymbolExport("EotR8a3ASf4", "pthread_self", "libScePosix", "libkernel", (void*)&kernel_pthread_self);
    module.addSymbolExport("aI+OeCz8xrQ", "scePthreadSelf", "libkernel", "libkernel", (void*)&kernel_pthread_self);
    module.addSymbolExport("onNY9Byn-W8", "scePthreadJoin", "libkernel", "libkernel", (void*)&kernel_pthread_join);

    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("9UK1vLZQft4", "scePthreadMutexLock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("K-jXhbt2gn4", "pthread_mutex_trylock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_trylock);
    module.addSymbolExport("upoVrzMHFeE", "scePthreadMutexTrylock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_trylock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("tn3VlD0hG60", "scePthreadMutexUnlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("dQHWEsJtoE4", "pthread_mutexattr_init", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("dQHWEsJtoE4", "pthread_mutexattr_init", "libScePosix", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("F8bUHwAG284", "scePthreadMutexattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("smWEktiyyG0", "scePthreadMutexattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_destroy);
    module.addSymbolExport("mDmgMOGVUqg", "pthread_mutexattr_settype", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("mDmgMOGVUqg", "pthread_mutexattr_settype", "libScePosix", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("iMp8QpE+XO4", "scePthreadMutexattrSettype", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("ttHNfU+qDBU", "pthread_mutex_init", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("ttHNfU+qDBU", "pthread_mutex_init", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("cmo1RIYva9o", "scePthreadMutexInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("2Of0f+3mhhE", "scePthreadMutexDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_destroy);

    module.addSymbolExport("mKoTx03HRWA", "pthread_condattr_init", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("m5-2bsNfv7s", "scePthreadCondattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("0TyVk4MSLt0", "pthread_cond_init", "libkernel", "libkernel", (void*)&kernel_pthread_cond_init);
    module.addSymbolExport("0TyVk4MSLt0", "pthread_cond_init", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_init);
    module.addSymbolExport("2Tb92quprl0", "scePthreadCondInit", "libkernel", "libkernel", (void*)&scePthreadCondInit);
    module.addSymbolExport("Op8TBGY5KHg", "pthread_cond_wait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("WKAXJ4XBPQ4", "scePthreadCondWait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("27bAgiJmOh0", "pthread_cond_timedwait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_timedwait);
    module.addSymbolExport("27bAgiJmOh0", "pthread_cond_timedwait", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_timedwait);
    module.addSymbolExport("BmMjYxmew1w", "scePthreadCondTimedwait", "libkernel", "libkernel", (void*)&scePthreadCondTimedwait);
    module.addSymbolExport("2MOy+rUfuhQ", "pthread_cond_signal", "libkernel", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("kDh-NfxgMtE", "scePthreadCondSignal", "libkernel", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("mkx2fVhNMsg", "pthread_cond_broadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("JGgj7Uvrl+A", "scePthreadCondBroadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("dJcuQVn6-Iw", "pthread_condattr_destroy", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    module.addSymbolExport("waPcxYiR3WA", "scePthreadCondattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    module.addSymbolStub("g+PZd2hiacg", "scePthreadCondDestroy", "libkernel", "libkernel");   // TODO
    
    module.addSymbolExport("Z4QosVuAsA0", "pthread_once", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("14bOACANTBo", "scePthreadOnce", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("0-KXaS70xy4", "pthread_getspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("eoht7mQOCmo", "scePthreadGetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("WrOLvHU0yQM", "pthread_setspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("+BzXYkqYeLE", "scePthreadSetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("mqULNdimTn0", "pthread_key_create", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("geDaqgH9lTg", "scePthreadKeyCreate", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("OxhIB8LB-PQ", "pthread_create", "libkernel", "libkernel", (void*)&kernel_pthread_create);
    module.addSymbolExport("OxhIB8LB-PQ", "pthread_create", "libScePosix", "libkernel", (void*)&kernel_pthread_create);
    module.addSymbolExport("6UgtwV+0zb4", "scePthreadCreate", "libkernel", "libkernel", (void*)&scePthreadCreate);
    module.addSymbolExport("+U1R4WtXvoc", "pthread_detach", "libkernel", "libkernel", (void*)&kernel_pthread_detach);
    module.addSymbolExport("4qGrR6eoP9Y", "scePthreadDetach", "libkernel", "libkernel", (void*)&kernel_pthread_detach);
    module.addSymbolExport("7Xl257M4VNI", "pthread_equal", "libkernel", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("3PtV6p3QNX4", "scePthreadEqual", "libkernel", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("B5GmVDKwpn0", "pthread_yield", "libkernel", "libkernel", (void*)&kernel_pthread_yield);
    module.addSymbolExport("T72hz6ffq08", "scePthreadYield", "libkernel", "libkernel", (void*)&kernel_pthread_yield);
    module.addSymbolExport("3kg7rT0NQIs", "scePthreadExit", "libkernel", "libkernel", (void*)&kernel_pthread_exit);
    
    module.addSymbolExport("9BcDykPmo1I", "__error", "libkernel", "libkernel", (void*)&kernel_error);
    module.addSymbolExport("f7uOxY9mM1U", "__stack_chk_guard", "libkernel", "libkernel", (void*)&stack_chk_guard);
    module.addSymbolExport("vNe1w4diLCs", "__tls_get_addr", "libkernel", "libkernel", (void*)&__tls_get_addr);
    module.addSymbolExport("BPE9s9vQQXo", "mmap", "libkernel", "libkernel", (void*)&kernel_mmap);

    module.addSymbolExport("6c3rCVE-fTU", "_open", "libkernel", "libkernel", (void*)&kernel_open);
    module.addSymbolExport("wuCroIGjt2g", "open", "libkernel", "libkernel", (void*)&kernel_open);   // ???
    module.addSymbolExport("1G3lF1Gg1k8", "sceKernelOpen", "libkernel", "libkernel", (void*)&sceKernelOpen);
    module.addSymbolExport("Oy6IpwgtYOk", "lseek", "libkernel", "libkernel", (void*)&kernel_lseek);
    module.addSymbolExport("Oy6IpwgtYOk", "lseek", "libScePosix", "libkernel", (void*)&kernel_lseek);
    module.addSymbolExport("oib76F-12fk", "sceKernelLseek", "libkernel", "libkernel", (void*)&sceKernelLseek);
    module.addSymbolExport("JGMio+21L4c", "mkdir", "libkernel", "libkernel", (void*)&kernel_mkdir);
    module.addSymbolExport("JGMio+21L4c", "mkdir", "libScePosix", "libkernel", (void*)&kernel_mkdir);
    module.addSymbolExport("JGMio+21L4c", "mkdir", "libScePosix", "libkernel", (void*)&kernel_mkdir);
    module.addSymbolExport("1-LFLmRFxxM", "sceKernelMkdir", "libkernel", "libkernel", (void*)&sceKernelMkdir);
    module.addSymbolExport("AqBioC2vF3I", "read", "libkernel", "libkernel", (void*)&kernel_read);
    module.addSymbolExport("AqBioC2vF3I", "read", "libScePosix", "libkernel", (void*)&kernel_read);
    module.addSymbolExport("Cg4srZ6TKbU", "sceKernelRead", "libkernel", "libkernel", (void*)&sceKernelRead);
    module.addSymbolExport("ezv-RSBNKqI", "pread", "libkernel", "libkernel", (void*)&kernel_pread);
    module.addSymbolExport("ezv-RSBNKqI", "pread", "libScePosix", "libkernel", (void*)&kernel_pread);
    module.addSymbolExport("+r3rMFwItV4", "sceKernelPread", "libkernel", "libkernel", (void*)&sceKernelPread);
    module.addSymbolExport("FxVZqBAA7ks", "_write", "libkernel", "libkernel", (void*)&kernel_write);
    module.addSymbolExport("4wSze92BhLI", "sceKernelWrite", "libkernel", "libkernel", (void*)&sceKernelWrite);
    module.addSymbolExport("YSHRBRLn2pI", "_writev", "libkernel", "libkernel", (void*)&kernel_writev);
    module.addSymbolExport("E6ao34wPw+U", "stat", "libkernel", "libkernel", (void*)&kernel_stat);
    module.addSymbolExport("E6ao34wPw+U", "stat", "libScePosix", "libkernel", (void*)&kernel_stat);
    module.addSymbolExport("eV9wAD2riIA", "sceKernelStat", "libkernel", "libkernel", (void*)&sceKernelStat);
    module.addSymbolExport("mqQMh1zPPT8", "fstat", "libkernel", "libkernel", (void*)&kernel_fstat);
    module.addSymbolExport("mqQMh1zPPT8", "fstat", "libScePosix", "libkernel", (void*)&kernel_fstat);
    module.addSymbolExport("kBwCPsYX-m4", "sceKernelFstat", "libkernel", "libkernel", (void*)&sceKernelFstat);
    module.addSymbolExport("bY-PO6JhzhQ", "close", "libkernel", "libkernel", (void*)&kernel_close);
    module.addSymbolExport("bY-PO6JhzhQ", "close", "libScePosix", "libkernel", (void*)&kernel_close);
    module.addSymbolExport("UK2Tl2DWUns", "sceKernelClose", "libkernel", "libkernel", (void*)&sceKernelClose);
    
    module.addSymbolExport("HoLVWNanBBc", "getpid", "libkernel", "libkernel", (void*)&kernel_getpid);
    module.addSymbolExport("CBNtXOoef-E", "sched_get_priority_max", "libkernel", "libkernel", (void*)&kernel_sched_get_priority_max);
    module.addSymbolExport("CBNtXOoef-E", "sched_get_priority_max", "libScePosix", "libkernel", (void*)&kernel_sched_get_priority_max);
    module.addSymbolExport("m0iS6jNsXds", "sched_get_priority_min", "libkernel", "libkernel", (void*)&kernel_sched_get_priority_min);
    module.addSymbolExport("m0iS6jNsXds", "sched_get_priority_min", "libScePosix", "libkernel", (void*)&kernel_sched_get_priority_min);
    module.addSymbolExport("VkTAsrZDcJ0", "sigfillset", "libkernel", "libkernel", (void*)&sigfillset);
    
    module.addSymbolExport("7NwggrWJ5cA", "__sys_regmgr_call", "libkernel", "libkernel", (void*)&__sys_regmgr_call);

    module.addSymbolExport("yS8U2TGCe1A", "nanosleep", "libkernel", "libkernel", (void*)&kernel_nanosleep);
    module.addSymbolExport("yS8U2TGCe1A", "nanosleep", "libScePosix", "libkernel", (void*)&kernel_nanosleep);
    module.addSymbolExport("QcteRwbsnV0", "usleep", "libkernel", "libkernel", (void*)&sceKernelUsleep); // TODO: Technically should be a separate function, but the behavior should be the same
    module.addSymbolExport("QcteRwbsnV0", "usleep", "libScePosix", "libkernel", (void*)&sceKernelUsleep); // TODO: Technically should be a separate function, but the behavior should be the same
    module.addSymbolExport("1jfXLRVzisc", "sceKernelUsleep", "libkernel", "libkernel", (void*)&sceKernelUsleep);
    module.addSymbolExport("-ZR+hG7aDHw", "sceKernelSleep", "libkernel", "libkernel", (void*)&sceKernelSleep);
    module.addSymbolExport("lLMT9vJAck0", "clock_gettime", "libkernel", "libkernel", (void*)&kernel_clock_gettime);
    module.addSymbolExport("lLMT9vJAck0", "clock_gettime", "libScePosix", "libkernel", (void*)&kernel_clock_gettime);
    module.addSymbolExport("QBi7HCK03hw", "sceKernelClockGettime", "libkernel", "libkernel", (void*)&sceKernelClockGettime);
    module.addSymbolExport("n88vx3C5nW8", "gettimeofday", "libkernel", "libkernel", (void*)&kernel_gettimeofday);
    module.addSymbolExport("n88vx3C5nW8", "gettimeofday", "libScePosix", "libkernel", (void*)&kernel_gettimeofday);
    module.addSymbolExport("ejekcaNQNq0", "sceKernelGettimeofday", "libkernel", "libkernel", (void*)&sceKernelGettimeofday);
    module.addSymbolExport("kOcnerypnQA", "sceKernelGettimezone", "libkernel", "libkernel", (void*)&sceKernelGettimezone);
    module.addSymbolExport("4J2sUJmuHZQ", "sceKernelGetProcessTime", "libkernel", "libkernel", (void*)&sceKernelGetProcessTime);
    module.addSymbolExport("-2IRUCO--PM", "sceKernelReadTsc", "libkernel", "libkernel", (void*)&sceKernelReadTsc);
    module.addSymbolExport("6XG4B33N09g", "sched_yield", "libkernel", "libkernel", (void*)&kernel_sched_yield);
    
    module.addSymbolExport("WslcK1FQcGI", "sceKernelIsNeoMode", "libkernel", "libkernel", (void*)&sceKernelIsNeoMode);
    module.addSymbolExport("959qrazPIrg", "sceKernelGetProcParam", "libkernel", "libkernel", (void*)&sceKernelGetProcParam);
    module.addSymbolExport("+g+UP8Pyfmo", "sceKernelGetProcessType", "libkernel", "libkernel", (void*)&sceKernelGetProcessType);
    module.addSymbolExport("p5EcQeEeJAE", "_sceKernelRtldSetApplicationHeapAPI", "libkernel", "libkernel", (void*)&_sceKernelRtldSetApplicationHeapAPI);
    module.addSymbolExport("1j3S3n-tTW4", "sceKernelGetTscFrequency", "libkernel", "libkernel", (void*)&sceKernelGetTscFrequency);

    module.addSymbolExport("D0OdFMjp46I", "sceKernelCreateEqueue", "libkernel", "libkernel", (void*)&sceKernelCreateEqueue);
    module.addSymbolExport("fzyMKs9kim0", "sceKernelWaitEqueue", "libkernel", "libkernel", (void*)&sceKernelWaitEqueue);
    module.addSymbolExport("4R6-OvI2cEA", "sceKernelAddUserEvent", "libkernel", "libkernel", (void*)&sceKernelAddUserEvent);
    module.addSymbolExport("23CPPI1tyBY", "sceKernelGetEventFilter", "libkernel", "libkernel", (void*)&sceKernelGetEventFilter);
    
    module.addSymbolExport("BpFoboUJoZU", "sceKernelCreateEventFlag", "libkernel", "libkernel", (void*)&sceKernelCreateEventFlag);
    module.addSymbolExport("IOnSvHzqu6A", "sceKernelSetEventFlag", "libkernel", "libkernel", (void*)&sceKernelSetEventFlag);
    module.addSymbolExport("7uhBFWRAS60", "sceKernelClearEventFlag", "libkernel", "libkernel", (void*)&sceKernelClearEventFlag);
    module.addSymbolExport("JTvBflhYazQ", "sceKernelWaitEventFlag", "libkernel", "libkernel", (void*)&sceKernelWaitEventFlag);
    module.addSymbolExport("9lvj5DjHZiA", "sceKernelPollEventFlag", "libkernel", "libkernel", (void*)&sceKernelPollEventFlag);
    
    module.addSymbolExport("188x57JYp0g", "sceKernelCreateSema", "libkernel", "libkernel", (void*)&sceKernelCreateSema);
    module.addSymbolExport("4czppHBiriw", "sceKernelSignalSema", "libkernel", "libkernel", (void*)&sceKernelSignalSema);
    module.addSymbolExport("Zxa0VhQVTsk", "sceKernelWaitSema", "libkernel", "libkernel", (void*)&sceKernelWaitSema);
    module.addSymbolExport("12wOHk8ywb0", "sceKernelPollSema", "libkernel", "libkernel", (void*)&sceKernelPollSema);
    module.addSymbolExport("pDuPEf3m4fI", "sem_init", "libkernel", "libkernel", (void*)&kernel_sem_init);
    module.addSymbolExport("pDuPEf3m4fI", "sem_init", "libScePosix", "libkernel", (void*)&kernel_sem_init);
    module.addSymbolExport("IKP8typ0QUk", "sem_post", "libkernel", "libkernel", (void*)&kernel_sem_post);
    module.addSymbolExport("IKP8typ0QUk", "sem_post", "libScePosix", "libkernel", (void*)&kernel_sem_post);
    module.addSymbolExport("YCV5dGGBcCo", "sem_wait", "libkernel", "libkernel", (void*)&kernel_sem_wait);
    module.addSymbolExport("YCV5dGGBcCo", "sem_wait", "libScePosix", "libkernel", (void*)&kernel_sem_wait);
    module.addSymbolStub("cDW233RAwWo", "sem_destroy", "libkernel", "libkernel");
    module.addSymbolStub("cDW233RAwWo", "sem_destroy", "libScePosix", "libkernel");
    
    module.addSymbolExport("6ULAa0fq4jA", "scePthreadRwlockInit", "libkernel", "libkernel", (void*)&scePthreadRwlockInit);
    module.addSymbolExport("Ox9i0c7L5w0", "scePthreadRwlockRdlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_rdlock);
    module.addSymbolExport("mqdNorrB+gI", "scePthreadRwlockWrlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_wrlock);
    module.addSymbolExport("+L98PIbGttk", "scePthreadRwlockUnlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_unlock);
    
    module.addSymbolExport("B+vc2AO2Zrc", "sceKernelAllocateMainDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateMainDirectMemory);
    module.addSymbolExport("rTXw65xmLIA", "sceKernelAllocateDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateDirectMemory);
    module.addSymbolExport("L-Q3LEjIbgA", "sceKernelMapDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapDirectMemory);
    module.addSymbolExport("NcaWUxfMNIQ", "sceKernelMapNamedDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapNamedDirectMemory);
    module.addSymbolExport("mL8NDH86iQI", "sceKernelMapNamedFlexibleMemory", "libkernel", "libkernel", (void*)&sceKernelMapNamedFlexibleMemory);
    module.addSymbolExport("cQke9UuBQOk", "sceKernelMunmap", "libkernel", "libkernel", (void*)&sceKernelMunmap);
    module.addSymbolExport("pO96TwzOm5E", "sceKernelGetDirectMemorySize", "libkernel", "libkernel", (void*)&sceKernelGetDirectMemorySize);
    
    module.addSymbolStub("ltCfaGr2JGE", "pthread_mutex_destroy", "libkernel", "libkernel");
    module.addSymbolStub("ltCfaGr2JGE", "pthread_mutex_destroy", "libScePosix", "libkernel");
    module.addSymbolStub("HF7lK46xzjY", "pthread_mutexattr_destroy", "libkernel", "libkernel");
    module.addSymbolStub("HF7lK46xzjY", "pthread_mutexattr_destroy", "libScePosix", "libkernel");
    module.addSymbolStub("euKRgm0Vn2M", "pthread_attr_setschedparam", "libkernel", "libkernel");
    module.addSymbolStub("euKRgm0Vn2M", "pthread_attr_setschedparam", "libScePosix", "libkernel");
    module.addSymbolStub("qlk9pSLsUmM", "pthread_attr_getschedparam", "libkernel", "libkernel");
    module.addSymbolStub("qlk9pSLsUmM", "pthread_attr_getschedparam", "libScePosix", "libkernel");
    module.addSymbolStub("8mql9OcQnd4", "sceKernelDeleteEventFlag", "libkernel", "libkernel");
    module.addSymbolStub("1FGvU0i9saQ", "scePthreadMutexattrSetprotocol", "libkernel", "libkernel");
    module.addSymbolStub("WB66evu8bsU", "sceKernelGetCompiledSdkVersion", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("vSMAm3cxYTY", "sceKernelMprotect", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("-o5uEDpN+oY", "sceKernelConvertUtcToLocaltime", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("0NTHN1NKONI", "sceKernelConvertLocaltimeToUtc", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("6xVpy0Fdq+I", "_sigprocmask", "libkernel", "libkernel");
    module.addSymbolStub("jh+8XiK4LeE", "sceKernelIsAddressSanitizerEnabled", "libkernel", "libkernel", false);
    module.addSymbolStub("bnZxYgAFeA0", "sceKernelGetSanitizerNewReplaceExternal", "libkernel", "libkernel");
    module.addSymbolStub("bt3CTBKmGyI", "scePthreadSetaffinity", "libkernel", "libkernel");
    module.addSymbolStub("rcrVFJsQWRY", "scePthreadGetaffinity", "libkernel", "libkernel");
    module.addSymbolStub("1tKyG7RlMJo", "scePthreadGetprio", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("W0Hpm2X0uPE", "scePthreadSetprio", "libkernel", "libkernel");
    module.addSymbolStub("eXbUSpEaTsA", "scePthreadAttrSetinheritsched", "libkernel", "libkernel");
    module.addSymbolStub("DzES9hQF4f4", "scePthreadAttrSetschedparam", "libkernel", "libkernel");
    module.addSymbolStub("4+h9EzwKF4I", "scePthreadAttrSetschedpolicy", "libkernel", "libkernel");
    module.addSymbolStub("3qxgM4ezETA", "scePthreadAttrSetaffinity", "libkernel", "libkernel");
    module.addSymbolStub("rNhWz+lvOMU", "_sceKernelSetThreadDtors", "libkernel", "libkernel");  // void
    module.addSymbolStub("pB-yGZ2nQ9o", "_sceKernelSetThreadAtexitCount", "libkernel", "libkernel");  // void
    module.addSymbolStub("WhCc1w3EhSI", "_sceKernelSetThreadAtexitReport", "libkernel", "libkernel");  // void
    module.addSymbolStub("DGMG3JshrZU", "sceKernelSetVirtualRangeName", "libkernel", "libkernel");
    module.addSymbolStub("PfccT7qURYE", "ioctl", "libkernel", "libkernel");
    module.addSymbolStub("TU-d9PfIHPM", "socket", "libkernel", "libkernel");
    module.addSymbolStub("TU-d9PfIHPM", "socket", "libScePosix", "libkernel");
    module.addSymbolStub("fFxGkxF2bVo", "setsockopt", "libkernel", "libkernel");
    module.addSymbolStub("fFxGkxF2bVo", "setsockopt", "libScePosix", "libkernel");
    module.addSymbolStub("T8fER+tIGgk", "select", "libkernel", "libkernel");
    module.addSymbolStub("T8fER+tIGgk", "select", "libScePosix", "libkernel");
    module.addSymbolStub("fZOeZIOEmLw", "send", "libKernel", "libkernel");
    module.addSymbolStub("fZOeZIOEmLw", "send", "libScePosix", "libkernel");
    module.addSymbolStub("TUuiYS2kE8s", "shutdown", "libkernel", "libkernel");
    module.addSymbolStub("TUuiYS2kE8s", "shutdown", "libScePosix", "libkernel");
}

static thread_local s32 posix_errno = 0;

#ifdef _WIN32
std::mutex allocator_mtx;

void* allocate(uptr reservation_start, uptr reservation_end, size_t size, size_t alignment) {
    auto lk = std::unique_lock<std::mutex>(allocator_mtx);

    if (reservation_start >= reservation_end) return nullptr;
    if (!alignment || (alignment & (alignment - 1)) != 0) return nullptr;
    if (size == 0) return nullptr;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    const size_t page_size = si.dwPageSize;
    alignment = alignment < page_size ? page_size : alignment;
    size = (size + page_size - 1) & ~(page_size - 1);

    uptr cur_addr = reservation_start;
    while (true) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery((void*)cur_addr, &mbi, sizeof(mbi)))
            Helpers::panic("allocate: VirtualQuery failed\n");

        if (mbi.State == MEM_RESERVE) {
            uptr region_end = (uptr)mbi.BaseAddress + mbi.RegionSize;
            if (cur_addr + size <= region_end) {
                // Try to commit memory
                void* ret = VirtualAlloc((void*)cur_addr, size, MEM_COMMIT, PAGE_READWRITE);
                if (ret) {
                    if ((u64)ret & (alignment - 1)) Helpers::panic("allocate: alignment error\n");

                    std::memset(ret, 0xcd, size);
                    return ret;
                }
            }

            // Free area wasn't big enough to allocate or VirtualAlloc failed
        }
        cur_addr = (uptr)mbi.BaseAddress + mbi.RegionSize;
        // Align up
        cur_addr = (cur_addr + alignment - 1) & ~(alignment - 1);
        
        if (cur_addr > reservation_end) Helpers::panic("allocate: out of memory\n");
    }

    
    return nullptr;
}
#endif

s32* PS4_FUNC kernel_error() {
    return &posix_errno;
}

void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx) {
    //log("__tls_get_addr(tls_idx=*%p)\n", tls_idx);
    //log("modid=%d, offset=0x%x\n", tls_idx->modid, tls_idx->offset);
    return (void*)((u64)Thread::getTLSPtr(tls_idx->modid) + tls_idx->offset);
}

void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs) {
    log("mmap(addr=%p, len=0x%llx, prot=0x%x, flags=0x%x, fd=%d, offs=0x%llx)\n", addr, len, prot, flags, fd, offs);
    // TODO: This is stubbed as malloc for now
    return std::malloc(len);
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

s32 PS4_FUNC sceKernelSleep(u32 s) {
    std::this_thread::sleep_for(std::chrono::seconds(s));
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
        const auto now = std::chrono::system_clock::now();
        const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);

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

s32 PS4_FUNC kernel_gettimeofday(SceKernelTimeval* tv, SceKernelTimezone* tz) {
    log("gettimeofday(tv=*%p, tz=*%p)\n", tv, tz);

    const auto now = std::chrono::system_clock::now();
    const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);

    if (tv) {
        tv->tv_sec = sec.time_since_epoch().count();
        tv->tv_nsec = ns.count();
    }
    if (tz) {
        tz->tz_dsttime = 0;
        tz->tz_minuteswest = 0;
    }
    return 0;
}

s32 PS4_FUNC sceKernelGettimeofday(SceKernelTimeval* tv) {
    log("sceKernelGettimeofday(tv=*%p)\n", tv);

    kernel_gettimeofday(tv, nullptr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGettimezone(SceKernelTimezone* tz) {
    log("sceKernelGettimezone(tz=*%p)\n", tz);

    kernel_gettimeofday(nullptr, tz);
    return SCE_OK;
}

static const auto process_start_time = std::chrono::steady_clock::now();
u64 PS4_FUNC sceKernelGetProcessTime() {
    //log("sceKernelGetProcessTime()\n");
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - process_start_time).count();
}

void PS4_FUNC kernel_sched_yield() {
    //std::this_thread::yield();
}

u64 PS4_FUNC sceKernelReadTsc() {
    log("sceKernelReadTsc()\n");
    // Stubbed to just be the same as sceKernelGetProcessTime
    return __rdtsc();
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

static u64 tsc_freq = 0;
u64 PS4_FUNC sceKernelGetTscFrequency() {
    log("sceKernelGetTscFrequency()\n");


#ifdef _WIN32
    if (!tsc_freq) {
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
                tsc_freq = i - first;
                printf_s("Measured frequency: %lld\n", tsc_freq);
                break;
            }
        }
    }
#else
    Helpers::panic("Unsupported platform\n");
#endif

    log("freq: %lld\n", tsc_freq);
    return tsc_freq;
}

s32 PS4_FUNC kernel_getpid() {
    log("getpid()\n");
    return 100;
}

s32 PS4_FUNC kernel_sched_get_priority_max() {
    log("sched_get_priority_max()\n");
    return 256;
}

s32 PS4_FUNC kernel_sched_get_priority_min() {
    log("sched_get_priority_min()\n");
    return 767;
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
    *out_addr = (void*)0x100000;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr) {
    log("sceKernelAllocateMainDirectMemory(size=0x%016llx, align=0x%016llx, mem_type=%d, out_addr=*%p)\n", size, align, mem_type, out_addr);

    // TODO: For now we allocate memory directly in the map function
    //       Eventually I will need to handle the physical memory map properly...
    *out_addr = (void*)0x100000;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align) {
    log("sceKernelMapDirectMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, dmem_start=0x%016llx, align=0x%016llx)\n", addr, len, prot, flags, dmem_start, align);

    void* in_addr = *addr;
    log("in_addr=%p\n", in_addr);

    align = align ? align : 16_KB;

    // TODO: prot, flags, verify align is a valid value (multiple of 16kb)
#ifdef _WIN32
    if (!in_addr) {
        *addr = allocate(0x8000'0000, 0x8000'0000 + 500_GB, len, align);
    }
    else if ((u64)in_addr >= 0x8000'0000)
        *addr = allocate((u64)in_addr, 0x8000'0000 + 500_GB, len, align);
    else
        // TODO
        *addr = allocate(0x8000'0000, 0x8000'0000 + 500_GB, len, align);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    if (!*addr) {
        Helpers::panic("sceKernelMapDirectMemory: failed to allocate\n");
    }

    // Clear allocated memory
    std::memset(*addr, 0, len);

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapNamedDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align, const char* name) {
    log("sceKernelMapNamedDirectMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, dmem_start=0x%016llx, align=0x%016llx, name=\"%s\")\n", addr, len, prot, flags, dmem_start, align, name);
    return sceKernelMapDirectMemory(addr, len, prot, flags, dmem_start, align);
}

s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name) {
    log("sceKernelMapNamedFlexibleMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, name=\"%s\")\n", addr, len, prot, flags, name);

    void* in_addr = *addr;
    if (in_addr) {
        Helpers::panic("TODO: sceKernelMapNamedFlexibleMemory with non-zero input addr\n");
    }

    // TODO: prot, flags
#ifdef _WIN32
    *addr = allocate(0x8000'0000, 0x8000'0000 + 500_GB, len, 16_KB);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    // Clear allocated memory
    std::memset(*addr, 0, len);

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMunmap(void* addr, size_t len) {
    log("sceKernelMunmap(addr=%p, len=0x%llx)\n", addr, len);

#ifdef _WIN32
    auto lk = std::unique_lock<std::mutex>(allocator_mtx);
    VirtualFree(addr, len, MEM_DECOMMIT);
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