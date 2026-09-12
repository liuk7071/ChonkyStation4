#include "Kernel.hpp"
#include <Logger.hpp>
#include <Profiler.hpp>
#include <ErrorCodes.hpp>
#include <NameToNid.hpp>
#include <Loaders/Module.hpp>
#include <Loaders/App.hpp>
#include <Loaders/Linker/Linker.hpp>
#include <OS/Thread.hpp>
#include <OS/Libraries/Kernel/pthread/pthread.hpp>
#include <OS/Libraries/Kernel/pthread/mutex.hpp>
#include <OS/Libraries/Kernel/pthread/cond.hpp>
#include <OS/Libraries/Kernel/pthread/rwlock.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
#include <OS/Libraries/Kernel/Eflag.hpp>
#include <OS/Libraries/Kernel/Semaphore.hpp>
#include <OS/Libraries/Kernel/Filesystem.hpp>
#include <OS/Filesystem.hpp>
#include <OS/SceObj.hpp>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <map>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#pragma intrinsic(__rdtsc)

#include <intrin.h>
#define RETURN_ADDRESS() _ReturnAddress()
#else
#define RETURN_ADDRESS() _builtin_return_address(0)
#endif
#include <SDL.h>    // For performance counters


extern App g_app;

namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);
MAKE_LOG_FUNCTION(log_force, force_enable);
MAKE_LOG_FUNCTION(unimpl, unimplemented);

static u64 stack_chk_guard = 0x4452474B43415453;    // "STACKGRD"
static u64 proc_counter_start = 0;
static char* environment[64];
static char** kernel_environ;

void init(Module& module) {
    module.addSymbolExport("wtkt-teR1so", "pthread_attr_init", "libkernel", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("wtkt-teR1so", "pthread_attr_init", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("nsYoNRywwNg", "scePthreadAttrInit", "libkernel", "libkernel", (void*)&kernel_pthread_attr_init);
    module.addSymbolExport("Ucsu-OK+els", "pthread_attr_get_np", "libkernel", "libkernel", (void*)&kernel_pthread_attr_get_np);
    module.addSymbolExport("x1X76arYMxU", "scePthreadAttrGet", "libkernel", "libkernel", (void*)&kernel_pthread_attr_get_np);
    module.addSymbolExport("-wzZ7dvA7UU", "pthread_attr_getaffinity_np", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getaffinity_np);
    module.addSymbolExport("8+s5BzZjxSg", "scePthreadAttrGetaffinity", "libkernel", "libkernel", (void*)&scePthreadAttrGetaffinity);
    module.addSymbolExport("vQm4fDEsWi8", "pthread_attr_getstack", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getstack);
    module.addSymbolExport("DxmIMUQ-wXY", "pthread_attr_getstackaddr", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getstackaddr);
    module.addSymbolExport("DxmIMUQ-wXY", "pthread_attr_getstackaddr", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_getstackaddr);
    module.addSymbolExport("0qOtCR-ZHck", "pthread_attr_getstacksize", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getstacksize);
    module.addSymbolExport("0qOtCR-ZHck", "pthread_attr_getstacksize", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_getstacksize);
    module.addSymbolExport("-quPa4SEJUw", "scePthreadAttrGetstack", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getstack);
    module.addSymbolExport("Ru36fiTtJzA", "scePthreadAttrGetstackaddr", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getstackaddr);
    module.addSymbolExport("-fA+7ZlGDQs", "scePthreadAttrGetstacksize", "libkernel", "libkernel", (void*)&kernel_pthread_attr_getstacksize);
    module.addSymbolExport("vQm4fDEsWi8", "pthread_attr_getstack", "libScePosix", "libkernel", (void*)&kernel_pthread_attr_getstack);
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
    module.addSymbolExport("h9CcP3J0oVM", "pthread_join", "libkernel", "libkernel", (void*)&kernel_pthread_join);
    module.addSymbolExport("h9CcP3J0oVM", "pthread_join", "libScePosix", "libkernel", (void*)&kernel_pthread_join);
    module.addSymbolExport("onNY9Byn-W8", "scePthreadJoin", "libkernel", "libkernel", (void*)&kernel_pthread_join);
    module.addSymbolStub("Bvn74vj6oLo", "scePthreadAttrSetstack", "libkernel", "libkernel");   // TODO: IMPORTANT!

    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("7H0iTOciTLo", "pthread_mutex_lock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("9UK1vLZQft4", "scePthreadMutexLock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_lock);
    module.addSymbolExport("K-jXhbt2gn4", "pthread_mutex_trylock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_trylock);
    module.addSymbolExport("K-jXhbt2gn4", "pthread_mutex_trylock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_trylock);
    module.addSymbolExport("upoVrzMHFeE", "scePthreadMutexTrylock", "libkernel", "libkernel", (void*)&scePthreadMutexTrylock);
    module.addSymbolExport("IafI2PxcPnQ", "scePthreadMutexTimedlock", "libkernel", "libkernel", (void*)&scePthreadMutexTimedlock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("2Z+PpY6CaJg", "pthread_mutex_unlock", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("tn3VlD0hG60", "scePthreadMutexUnlock", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_unlock);
    module.addSymbolExport("dQHWEsJtoE4", "pthread_mutexattr_init", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("dQHWEsJtoE4", "pthread_mutexattr_init", "libScePosix", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("F8bUHwAG284", "scePthreadMutexattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("n2MMpvU8igI", "scePthreadMutexattrInitForInternalLibc", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_init);
    module.addSymbolExport("smWEktiyyG0", "scePthreadMutexattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_destroy);
    module.addSymbolExport("mDmgMOGVUqg", "pthread_mutexattr_settype", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("mDmgMOGVUqg", "pthread_mutexattr_settype", "libScePosix", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("iMp8QpE+XO4", "scePthreadMutexattrSettype", "libkernel", "libkernel", (void*)&kernel_pthread_mutexattr_settype);
    module.addSymbolExport("ttHNfU+qDBU", "pthread_mutex_init", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("ttHNfU+qDBU", "pthread_mutex_init", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("cmo1RIYva9o", "scePthreadMutexInit", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("qH1gXoq71RY", "scePthreadMutexInitForInternalLibc", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_init);
    module.addSymbolExport("ltCfaGr2JGE", "pthread_mutex_destroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_destroy);
    module.addSymbolExport("ltCfaGr2JGE", "pthread_mutex_destroy", "libScePosix", "libkernel", (void*)&kernel_pthread_mutex_destroy);
    module.addSymbolExport("2Of0f+3mhhE", "scePthreadMutexDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_mutex_destroy);

    module.addSymbolExport("mKoTx03HRWA", "pthread_condattr_init", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("mKoTx03HRWA", "pthread_condattr_init", "libScePosix", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("m5-2bsNfv7s", "scePthreadCondattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_init);
    module.addSymbolExport("0TyVk4MSLt0", "pthread_cond_init", "libkernel", "libkernel", (void*)&kernel_pthread_cond_init);
    module.addSymbolExport("0TyVk4MSLt0", "pthread_cond_init", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_init);
    module.addSymbolExport("2Tb92quprl0", "scePthreadCondInit", "libkernel", "libkernel", (void*)&scePthreadCondInit);
    module.addSymbolExport("Op8TBGY5KHg", "pthread_cond_wait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("Op8TBGY5KHg", "pthread_cond_wait", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("WKAXJ4XBPQ4", "scePthreadCondWait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_wait);
    module.addSymbolExport("27bAgiJmOh0", "pthread_cond_timedwait", "libkernel", "libkernel", (void*)&kernel_pthread_cond_timedwait);
    module.addSymbolExport("27bAgiJmOh0", "pthread_cond_timedwait", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_timedwait);
    module.addSymbolExport("BmMjYxmew1w", "scePthreadCondTimedwait", "libkernel", "libkernel", (void*)&scePthreadCondTimedwait);
    module.addSymbolExport("K953PF5u6Pc", "pthread_cond_reltimedwait_np", "libkernel", "libkernel", (void*)&kernel_pthread_cond_reltimedwait_np);
    module.addSymbolExport("K953PF5u6Pc", "pthread_cond_reltimedwait_np", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_reltimedwait_np);
    module.addSymbolExport("2MOy+rUfuhQ", "pthread_cond_signal", "libkernel", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("2MOy+rUfuhQ", "pthread_cond_signal", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("kDh-NfxgMtE", "scePthreadCondSignal", "libkernel", "libkernel", (void*)&kernel_pthread_cond_signal);
    module.addSymbolExport("mkx2fVhNMsg", "pthread_cond_broadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("mkx2fVhNMsg", "pthread_cond_broadcast", "libScePosix", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("JGgj7Uvrl+A", "scePthreadCondBroadcast", "libkernel", "libkernel", (void*)&kernel_pthread_cond_broadcast);
    module.addSymbolExport("dJcuQVn6-Iw", "pthread_condattr_destroy", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    module.addSymbolExport("dJcuQVn6-Iw", "pthread_condattr_destroy", "libScePosix", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    module.addSymbolExport("waPcxYiR3WA", "scePthreadCondattrDestroy", "libkernel", "libkernel", (void*)&kernel_pthread_condattr_destroy);
    module.addSymbolStub("RXXqi4CtF8w", "pthread_cond_destroy", "libkernel", "libkernel");   // TODO
    module.addSymbolStub("RXXqi4CtF8w", "pthread_cond_destroy", "libScePosix", "libkernel");   // TODO
    module.addSymbolStub("g+PZd2hiacg", "scePthreadCondDestroy", "libkernel", "libkernel");   // TODO
    
    module.addSymbolExport("Z4QosVuAsA0", "pthread_once", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("Z4QosVuAsA0", "pthread_once", "libScePosix", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("14bOACANTBo", "scePthreadOnce", "libkernel", "libkernel", (void*)&kernel_pthread_once);
    module.addSymbolExport("0-KXaS70xy4", "pthread_getspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("0-KXaS70xy4", "pthread_getspecific", "libScePosix", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("eoht7mQOCmo", "scePthreadGetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_getspecific);
    module.addSymbolExport("WrOLvHU0yQM", "pthread_setspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("WrOLvHU0yQM", "pthread_setspecific", "libScePosix", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("+BzXYkqYeLE", "scePthreadSetspecific", "libkernel", "libkernel", (void*)&kernel_pthread_setspecific);
    module.addSymbolExport("mqULNdimTn0", "pthread_key_create", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("mqULNdimTn0", "pthread_key_create", "libScePosix", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("geDaqgH9lTg", "scePthreadKeyCreate", "libkernel", "libkernel", (void*)&kernel_pthread_key_create);
    module.addSymbolExport("OxhIB8LB-PQ", "pthread_create", "libkernel", "libkernel", (void*)&kernel_pthread_create);
    module.addSymbolExport("OxhIB8LB-PQ", "pthread_create", "libScePosix", "libkernel", (void*)&kernel_pthread_create);
    module.addSymbolExport("Jmi+9w9u0E4", "pthread_create_name_np", "libkernel", "libkernel", (void*)&scePthreadCreate);
    module.addSymbolExport("Jmi+9w9u0E4", "pthread_create_name_np", "libScePosix", "libkernel", (void*)&scePthreadCreate);
    module.addSymbolExport("6UgtwV+0zb4", "scePthreadCreate", "libkernel", "libkernel", (void*)&scePthreadCreate);
    module.addSymbolExport("9vyP6Z7bqzc", "pthread_rename_np", "libkernel", "libkernel", (void*)&scePthreadRename);
    module.addSymbolExport("GBUY7ywdULE", "scePthreadRename", "libkernel", "libkernel", (void*)&scePthreadRename);
    module.addSymbolExport("How7B8Oet6k", "scePthreadGetname", "libkernel", "libkernel", (void*)&scePthreadGetname);
    module.addSymbolExport("+U1R4WtXvoc", "pthread_detach", "libkernel", "libkernel", (void*)&kernel_pthread_detach);
    module.addSymbolExport("4qGrR6eoP9Y", "scePthreadDetach", "libkernel", "libkernel", (void*)&kernel_pthread_detach);
    module.addSymbolExport("7Xl257M4VNI", "pthread_equal", "libkernel", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("7Xl257M4VNI", "pthread_equal", "libScePosix", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("3PtV6p3QNX4", "scePthreadEqual", "libkernel", "libkernel", (void*)&kernel_pthread_equal);
    module.addSymbolExport("B5GmVDKwpn0", "pthread_yield", "libkernel", "libkernel", (void*)&kernel_pthread_yield);
    module.addSymbolExport("B5GmVDKwpn0", "pthread_yield", "libScePosix", "libkernel", (void*)&kernel_pthread_yield);
    module.addSymbolExport("T72hz6ffq08", "scePthreadYield", "libkernel", "libkernel", (void*)&kernel_pthread_yield);
    module.addSymbolExport("FJrT5LuUBAU", "pthread_exit", "libkernel", "libkernel", (void*)&kernel_pthread_exit);
    module.addSymbolExport("FJrT5LuUBAU", "pthread_exit", "libScePosix", "libkernel", (void*)&kernel_pthread_exit);
    module.addSymbolExport("3kg7rT0NQIs", "scePthreadExit", "libkernel", "libkernel", (void*)&kernel_pthread_exit);
    module.addSymbolExport("3eqs37G74-s", "pthread_getthreadid_np", "libkernel", "libkernel", (void*)&kernel_pthread_getthreadid_np);
    module.addSymbolExport("EI-5-jlq2dE", "scePthreadGetthreadid", "libkernel", "libkernel", (void*)&kernel_pthread_getthreadid_np);
    
    module.addSymbolExport("9BcDykPmo1I", "__error", "libkernel", "libkernel", (void*)&kernel_error);
    module.addSymbolExport("k+AXqu2-eBc", "getpagesize", "libkernel", "libkernel", (void*)&kernel_getpagesize);
    module.addSymbolExport("k+AXqu2-eBc", "getpagesize", "libScePosix", "libkernel", (void*)&kernel_getpagesize);
    module.addSymbolExport("f7uOxY9mM1U", "__stack_chk_guard", "libkernel", "libkernel", (void*)&stack_chk_guard);
    module.addSymbolExport("vNe1w4diLCs", "__tls_get_addr", "libkernel", "libkernel", (void*)&__tls_get_addr);
    module.addSymbolExport("+2thxYZ4syk", "environ", "libkernel", "libkernel", (void*)&kernel_environ);

    module.addSymbolExport("6c3rCVE-fTU", "_open", "libkernel", "libkernel", (void*)&kernel_open);
    module.addSymbolExport("wuCroIGjt2g", "open", "libkernel", "libkernel", (void*)&kernel_open);   // ???
    module.addSymbolExport("wuCroIGjt2g", "open", "libScePosix", "libkernel", (void*)&kernel_open);   // ???
    module.addSymbolExport("1G3lF1Gg1k8", "sceKernelOpen", "libkernel", "libkernel", (void*)&sceKernelOpen);
    module.addSymbolExport("uWyW3v98sU4", "sceKernelCheckReachability", "libkernel", "libkernel", (void*)&sceKernelCheckReachability);
    module.addSymbolExport("Oy6IpwgtYOk", "lseek", "libkernel", "libkernel", (void*)&kernel_lseek);
    module.addSymbolExport("Oy6IpwgtYOk", "lseek", "libScePosix", "libkernel", (void*)&kernel_lseek);
    module.addSymbolExport("oib76F-12fk", "sceKernelLseek", "libkernel", "libkernel", (void*)&sceKernelLseek);
    module.addSymbolExport("JGMio+21L4c", "mkdir", "libkernel", "libkernel", (void*)&kernel_mkdir);
    module.addSymbolExport("JGMio+21L4c", "mkdir", "libScePosix", "libkernel", (void*)&kernel_mkdir);
    module.addSymbolExport("JGMio+21L4c", "mkdir", "libScePosix", "libkernel", (void*)&kernel_mkdir);
    module.addSymbolExport("1-LFLmRFxxM", "sceKernelMkdir", "libkernel", "libkernel", (void*)&sceKernelMkdir);
    module.addSymbolStub("c7ZnT7V1B98", "rmdir", "libkernel", "libkernel");
    module.addSymbolStub("c7ZnT7V1B98", "rmdir", "libScePosix", "libkernel");
    module.addSymbolExport("AqBioC2vF3I", "read", "libkernel", "libkernel", (void*)&kernel_read);
    module.addSymbolExport("AqBioC2vF3I", "read", "libScePosix", "libkernel", (void*)&kernel_read);
    module.addSymbolExport("DRuBt2pvICk", "_read", "libkernel", "libkernel", (void*)&kernel_read);
    module.addSymbolExport("Cg4srZ6TKbU", "sceKernelRead", "libkernel", "libkernel", (void*)&sceKernelRead);
    module.addSymbolExport("ezv-RSBNKqI", "pread", "libkernel", "libkernel", (void*)&kernel_pread);
    module.addSymbolExport("ezv-RSBNKqI", "pread", "libScePosix", "libkernel", (void*)&kernel_pread);
    module.addSymbolExport("+r3rMFwItV4", "sceKernelPread", "libkernel", "libkernel", (void*)&sceKernelPread);
    module.addSymbolExport("+WRlkKjZvag", "_readv", "libkernel", "libkernel", (void*)&kernel_readv);
    module.addSymbolExport("+WRlkKjZvag", "_readv", "libScePosix", "libkernel", (void*)&kernel_readv);
    module.addSymbolExport("pd02UI9TbOA", "readlink", "libkernel", "libkernel", (void*)&kernel_readlink);
    module.addSymbolExport("FxVZqBAA7ks", "_write", "libkernel", "libkernel", (void*)&kernel_write);
    module.addSymbolExport("4wSze92BhLI", "sceKernelWrite", "libkernel", "libkernel", (void*)&sceKernelWrite);
    module.addSymbolExport("C2kJ-byS5rM", "pwrite", "libkernel", "libkernel", (void*)&kernel_pwrite);
    module.addSymbolExport("C2kJ-byS5rM", "pwrite", "libScePosix", "libkernel", (void*)&kernel_pwrite);
    module.addSymbolExport("nKWi-N2HBV4", "sceKernelPwrite", "libkernel", "libkernel", (void*)&sceKernelPwrite);
    module.addSymbolExport("YSHRBRLn2pI", "_writev", "libkernel", "libkernel", (void*)&kernel_writev);
    module.addSymbolExport("ih4CD9-gghM", "ftruncate", "libkernel", "libkernel", (void*)&kernel_ftruncate);
    module.addSymbolExport("ih4CD9-gghM", "ftruncate", "libScePosix", "libkernel", (void*)&kernel_ftruncate);
    module.addSymbolExport("VW3TVZiM4-E", "sceKernelFtruncate", "libkernel", "libkernel", (void*)&sceKernelFtruncate);
    module.addSymbolExport("E6ao34wPw+U", "stat", "libkernel", "libkernel", (void*)&kernel_stat);
    module.addSymbolExport("E6ao34wPw+U", "stat", "libScePosix", "libkernel", (void*)&kernel_stat);
    module.addSymbolExport("eV9wAD2riIA", "sceKernelStat", "libkernel", "libkernel", (void*)&sceKernelStat);
    module.addSymbolExport("DRGXpDDh8Ng", "lstat", "libkernel", "libkernel", (void*)&kernel_stat);  // TODO: symlinks
    module.addSymbolExport("mqQMh1zPPT8", "fstat", "libkernel", "libkernel", (void*)&kernel_fstat);
    module.addSymbolExport("mqQMh1zPPT8", "fstat", "libScePosix", "libkernel", (void*)&kernel_fstat);
    module.addSymbolExport("kBwCPsYX-m4", "sceKernelFstat", "libkernel", "libkernel", (void*)&sceKernelFstat);
    module.addSymbolExport("2G6i6hMIUUY", "getdents", "libkernel", "libkernel", (void*)&kernel_getdents);
    module.addSymbolExport("2G6i6hMIUUY", "getdents", "libScePosix", "libkernel", (void*)&kernel_getdents);
    module.addSymbolExport("j2AIqSqJP0w", "sceKernelGetdents", "libkernel", "libkernel", (void*)&sceKernelGetdents);
    module.addSymbolExport("sfKygSjIbI8", "_getdirentries", "libkernel", "libkernel", (void*)&kernel_getdirentries);
    module.addSymbolExport("sfKygSjIbI8", "_getdirentries", "libScePosix", "libkernel", (void*)&kernel_getdirentries);
    module.addSymbolExport("taRWhTJFTgE", "sceKernelGetdirentries", "libkernel", "libkernel", (void*)&sceKernelGetdirentries);
    module.addSymbolExport("bY-PO6JhzhQ", "close", "libkernel", "libkernel", (void*)&kernel_close);
    module.addSymbolExport("bY-PO6JhzhQ", "close", "libScePosix", "libkernel", (void*)&kernel_close);
    module.addSymbolExport("NNtFaKJbPt0", "_close", "libkernel", "libkernel", (void*)&kernel_close);
    module.addSymbolExport("UK2Tl2DWUns", "sceKernelClose", "libkernel", "libkernel", (void*)&sceKernelClose);
    module.addSymbolExport("6mMQ1MSPW-Q", "chdir", "libkernel", "libkernel", (void*)&kernel_chdir);
    module.addSymbolExport("JGfTMBOdUJo", "sceKernelGetFsSandboxRandomWord", "libkernel", "libkernel", (void*)&sceKernelGetFsSandboxRandomWord);
    module.addSymbolStub("fTx66l5iWIA", "sceKernelFsync", "libkernel", "libkernel");
    module.addSymbolStub("naInUjYt3so", "sceKernelRmdir", "libkernel", "libkernel");
    
    module.addSymbolExport("HoLVWNanBBc", "getpid", "libkernel", "libkernel", (void*)&kernel_getpid);
    module.addSymbolExport("HoLVWNanBBc", "getpid", "libScePosix", "libkernel", (void*)&kernel_getpid);
    module.addSymbolExport("CBNtXOoef-E", "sched_get_priority_max", "libkernel", "libkernel", (void*)&kernel_sched_get_priority_max);
    module.addSymbolExport("CBNtXOoef-E", "sched_get_priority_max", "libScePosix", "libkernel", (void*)&kernel_sched_get_priority_max);
    module.addSymbolExport("m0iS6jNsXds", "sched_get_priority_min", "libkernel", "libkernel", (void*)&kernel_sched_get_priority_min);
    module.addSymbolExport("m0iS6jNsXds", "sched_get_priority_min", "libScePosix", "libkernel", (void*)&kernel_sched_get_priority_min);
    module.addSymbolExport("VkTAsrZDcJ0", "sigfillset", "libkernel", "libkernel", (void*)&sigfillset);
    module.addSymbolExport("9JYNqN6jAKI", "sceKernelDebugOutText", "libkernel", "libkernel", (void*)&sceKernelDebugOutText);
    
    module.addSymbolExport("7NwggrWJ5cA", "__sys_regmgr_call", "libkernel", "libkernel", (void*)&__sys_regmgr_call);

    module.addSymbolExport("yS8U2TGCe1A", "nanosleep", "libkernel", "libkernel", (void*)&kernel_nanosleep);
    module.addSymbolExport("yS8U2TGCe1A", "nanosleep", "libScePosix", "libkernel", (void*)&kernel_nanosleep);
    module.addSymbolExport("QcteRwbsnV0", "usleep", "libkernel", "libkernel", (void*)&sceKernelUsleep); // TODO: Technically should be a separate function, but the behavior should be the same
    module.addSymbolExport("QcteRwbsnV0", "usleep", "libScePosix", "libkernel", (void*)&sceKernelUsleep); // TODO: Technically should be a separate function, but the behavior should be the same
    module.addSymbolExport("QvsZxomvUHs", "sceKernelNanosleep", "libkernel", "libkernel", (void*)&sceKernelNanosleep);
    module.addSymbolExport("1jfXLRVzisc", "sceKernelUsleep", "libkernel", "libkernel", (void*)&sceKernelUsleep);
    module.addSymbolExport("-ZR+hG7aDHw", "sceKernelSleep", "libkernel", "libkernel", (void*)&sceKernelSleep);
    module.addSymbolExport("lLMT9vJAck0", "clock_gettime", "libkernel", "libkernel", (void*)&kernel_clock_gettime);
    module.addSymbolExport("lLMT9vJAck0", "clock_gettime", "libScePosix", "libkernel", (void*)&kernel_clock_gettime);
    module.addSymbolExport("QBi7HCK03hw", "sceKernelClockGettime", "libkernel", "libkernel", (void*)&sceKernelClockGettime);
    module.addSymbolExport("n88vx3C5nW8", "gettimeofday", "libkernel", "libkernel", (void*)&kernel_gettimeofday);
    module.addSymbolExport("n88vx3C5nW8", "gettimeofday", "libScePosix", "libkernel", (void*)&kernel_gettimeofday);
    module.addSymbolExport("ejekcaNQNq0", "sceKernelGettimeofday", "libkernel", "libkernel", (void*)&sceKernelGettimeofday);
    module.addSymbolExport("kOcnerypnQA", "sceKernelGettimezone", "libkernel", "libkernel", (void*)&sceKernelGettimezone);
    module.addSymbolExport("-o5uEDpN+oY", "sceKernelConvertUtcToLocaltime", "libkernel", "libkernel", (void*)&sceKernelConvertUtcToLocaltime);
    module.addSymbolExport("0NTHN1NKONI", "sceKernelConvertLocaltimeToUtc", "libkernel", "libkernel", (void*)&sceKernelConvertLocaltimeToUtc);
    module.addSymbolExport("4J2sUJmuHZQ", "sceKernelGetProcessTime", "libkernel", "libkernel", (void*)&sceKernelGetProcessTime);
    module.addSymbolExport("fgxnMeTNUtY", "sceKernelGetProcessTimeCounter", "libkernel", "libkernel", (void*)&sceKernelGetProcessTimeCounter);
    module.addSymbolExport("BNowx2l588E", "sceKernelGetProcessTimeCounterFrequency", "libkernel", "libkernel", (void*)&sceKernelGetProcessTimeCounterFrequency);
    module.addSymbolExport("-2IRUCO--PM", "sceKernelReadTsc", "libkernel", "libkernel", (void*)&sceKernelReadTsc);
    module.addSymbolExport("6XG4B33N09g", "sched_yield", "libkernel", "libkernel", (void*)&kernel_sched_yield);
    module.addSymbolExport("6XG4B33N09g", "sched_yield", "libScePosix", "libkernel", (void*)&kernel_sched_yield);
    
    module.addSymbolExport("WslcK1FQcGI", "sceKernelIsNeoMode", "libkernel", "libkernel", (void*)&sceKernelIsNeoMode);
    module.addSymbolStub("rNRtm1uioyY", "sceKernelHasNeoMode", "libkernel", "libkernel", false);
    module.addSymbolExport("959qrazPIrg", "sceKernelGetProcParam", "libkernel", "libkernel", (void*)&sceKernelGetProcParam);
    module.addSymbolExport("+g+UP8Pyfmo", "sceKernelGetProcessType", "libkernel", "libkernel", (void*)&sceKernelGetProcessType);
    module.addSymbolExport("p5EcQeEeJAE", "_sceKernelRtldSetApplicationHeapAPI", "libkernel", "libkernel", (void*)&_sceKernelRtldSetApplicationHeapAPI);
    module.addSymbolExport("1j3S3n-tTW4", "sceKernelGetTscFrequency", "libkernel", "libkernel", (void*)&sceKernelGetTscFrequency);
    module.addSymbolExport("G-MYv5erXaU", "sceKernelGetAppInfo", "libkernel", "libkernel", (void*)&sceKernelGetAppInfo);
    module.addSymbolExport("1yca4VvfcNA", "sceKernelTitleWorkaroundIsEnabled", "libkernel", "libkernel", (void*)&sceKernelTitleWorkaroundIsEnabled);
    module.addSymbolExport("Mv1zUObHvXI", "sceKernelGetSystemSwVersion", "libkernel", "libkernel", (void*)&sceKernelGetSystemSwVersion);
    module.addSymbolExport("QgsKEUfkqMA", "sceKernelGetModuleInfo2", "libkernel_module_info", "libkernel", (void*)&sceKernelGetModuleInfo2);
    module.addSymbolExport("ZzzC3ZGVAkc", "sceKernelGetModuleList2", "libkernel_module_info", "libkernel", (void*)&sceKernelGetModuleList2);
    module.addSymbolExport("f7KBOafysXo", "sceKernelGetModuleInfoFromAddr", "libkernel", "libkernel", (void*)&sceKernelGetModuleInfoFromAddr);
    module.addSymbolExport("RpQJJVKTiFM", "sceKernelGetModuleInfoForUnwind", "libkernel", "libkernel", (void*)&sceKernelGetModuleInfoForUnwind);
    module.addSymbolExport("zE-wXIZjLoM", "sceKernelDebugRaiseExceptionOnReleaseMode", "libkernel", "libkernel", (void*)&sceKernelDebugRaiseExceptionOnReleaseMode);
    module.addSymbolExport("Wh7HbV7JFqc", "getrlimit", "libkernel", "libkernel", (void*)&kernel_getrlimit);
    module.addSymbolExport("Wh7HbV7JFqc", "getrlimit", "libScePosix", "libkernel", (void*)&kernel_getrlimit);
    module.addSymbolExport("pi90NsG3zPA", "sceLibcMspaceCreateForMonoMutex", "libkernel", "libkernel", (void*)&sceLibcMspaceCreateForMonoMutex);
    module.addSymbolExport("WB66evu8bsU", "sceKernelGetCompiledSdkVersion", "libkernel", "libkernel", (void*)&sceKernelGetCompiledSdkVersion);
    module.addSymbolExport("fUJRLEbJOuQ", "sceKernelGetProcessName", "libkernel", "libkernel", (void*)&sceKernelGetProcessName);
    module.addSymbolExport("i-H8tE6wTqI", "sceKernelGetDataTransferMode", "libkernel", "libkernel", (void*)&sceKernelGetDataTransferMode);
    module.addSymbolExport("QtLhuYZf9jg", "sceKernelGetBackupRestoreMode", "libkernel", "libkernel", (void*)&sceKernelGetBackupRestoreMode);
    module.addSymbolExport("iKJMWrAumPE", "getargc", "libkernel", "libkernel", (void*)&kernel_getargc);
    module.addSymbolExport("FJmglmTMdr4", "getargc", "libkernel", "libkernel", (void*)&kernel_getargv);

    module.addSymbolExport("D0OdFMjp46I", "sceKernelCreateEqueue", "libkernel", "libkernel", (void*)&sceKernelCreateEqueue);
    module.addSymbolExport("fzyMKs9kim0", "sceKernelWaitEqueue", "libkernel", "libkernel", (void*)&sceKernelWaitEqueue);
    module.addSymbolExport("4R6-OvI2cEA", "sceKernelAddUserEvent", "libkernel", "libkernel", (void*)&sceKernelAddUserEvent);
    module.addSymbolExport("57ZK+ODEXWY", "sceKernelAddTimerEvent", "libkernel", "libkernel", (void*)&sceKernelAddTimerEvent);
    module.addSymbolExport("R74tt43xP6k", "sceKernelAddHRTimerEvent", "libkernel", "libkernel", (void*)&sceKernelAddHRTimerEvent);
    module.addSymbolStub("J+LF6LwObXU", "sceKernelDeleteHRTimerEvent", "libkernel", "libkernel");
    module.addSymbolStub("WDszmSbWuDk", "sceKernelAddUserEventEdge", "libkernel", "libkernel");
    module.addSymbolExport("23CPPI1tyBY", "sceKernelGetEventFilter", "libkernel", "libkernel", (void*)&sceKernelGetEventFilter);
    module.addSymbolExport("nh2IFMgKTv8", "kqueue", "libkernel", "libkernel", (void*)&kernel_kqueue);
    module.addSymbolExport("nh2IFMgKTv8", "kqueue", "libScePosix", "libkernel", (void*)&kernel_kqueue);
    module.addSymbolExport("RW-GEfpnsqg", "kevent", "libkernel", "libkernel", (void*)&kernel_kevent);
    module.addSymbolExport("RW-GEfpnsqg", "kevent", "libScePosix", "libkernel", (void*)&kernel_kevent);
    module.addSymbolStub("jpFjmgAC5AE", "sceKernelDeleteEqueue", "libkernel", "libkernel");
    
    module.addSymbolExport("BpFoboUJoZU", "sceKernelCreateEventFlag", "libkernel", "libkernel", (void*)&sceKernelCreateEventFlag);
    module.addSymbolExport("1vDaenmJtyA", "sceKernelOpenEventFlag", "libkernel", "libkernel", (void*)&sceKernelOpenEventFlag);
    module.addSymbolStub("s9-RaxukuzQ", "sceKernelCloseEventFlag", "libkernel", "libkernel");
    module.addSymbolExport("IOnSvHzqu6A", "sceKernelSetEventFlag", "libkernel", "libkernel", (void*)&sceKernelSetEventFlag);
    module.addSymbolExport("7uhBFWRAS60", "sceKernelClearEventFlag", "libkernel", "libkernel", (void*)&sceKernelClearEventFlag);
    module.addSymbolExport("JTvBflhYazQ", "sceKernelWaitEventFlag", "libkernel", "libkernel", (void*)&sceKernelWaitEventFlag);
    module.addSymbolExport("9lvj5DjHZiA", "sceKernelPollEventFlag", "libkernel", "libkernel", (void*)&sceKernelPollEventFlag);
    
    module.addSymbolExport("188x57JYp0g", "sceKernelCreateSema", "libkernel", "libkernel", (void*)&sceKernelCreateSema);
    module.addSymbolExport("4czppHBiriw", "sceKernelSignalSema", "libkernel", "libkernel", (void*)&sceKernelSignalSema);
    module.addSymbolExport("Zxa0VhQVTsk", "sceKernelWaitSema", "libkernel", "libkernel", (void*)&sceKernelWaitSema);
    module.addSymbolExport("12wOHk8ywb0", "sceKernelPollSema", "libkernel", "libkernel", (void*)&sceKernelPollSema);
    module.addSymbolExport("4DM06U2BNEY", "sceKernelCancelSema", "libkernel", "libkernel", (void*)&sceKernelCancelSema);
    module.addSymbolExport("R1Jvn8bSCW8", "sceKernelDeleteSema", "libkernel", "libkernel", (void*)&sceKernelDeleteSema);
    module.addSymbolExport("pDuPEf3m4fI", "sem_init", "libkernel", "libkernel", (void*)&kernel_sem_init);
    module.addSymbolExport("pDuPEf3m4fI", "sem_init", "libScePosix", "libkernel", (void*)&kernel_sem_init);
    module.addSymbolExport("IKP8typ0QUk", "sem_post", "libkernel", "libkernel", (void*)&kernel_sem_post);
    module.addSymbolExport("IKP8typ0QUk", "sem_post", "libScePosix", "libkernel", (void*)&kernel_sem_post);
    module.addSymbolExport("YCV5dGGBcCo", "sem_wait", "libkernel", "libkernel", (void*)&kernel_sem_wait);
    module.addSymbolExport("YCV5dGGBcCo", "sem_wait", "libScePosix", "libkernel", (void*)&kernel_sem_wait);
    module.addSymbolExport("WBWzsRifCEA", "sem_trywait", "libkernel", "libkernel", (void*)&kernel_sem_trywait);
    module.addSymbolExport("WBWzsRifCEA", "sem_trywait", "libScePosix", "libkernel", (void*)&kernel_sem_trywait);
    module.addSymbolExport("w5IHyvahg-o", "sem_timedwait", "libkernel", "libkernel", (void*)&kernel_sem_timedwait);
    module.addSymbolExport("w5IHyvahg-o", "sem_timedwait", "libScePosix", "libkernel", (void*)&kernel_sem_timedwait);
    module.addSymbolExport("Bq+LRV-N6Hk", "sem_getvalue", "libkernel", "libkernel", (void*)&kernel_sem_getvalue);
    module.addSymbolExport("Bq+LRV-N6Hk", "sem_getvalue", "libScePosix", "libkernel", (void*)&kernel_sem_getvalue);
    module.addSymbolExport("cDW233RAwWo", "sem_destroy", "libkernel", "libkernel", (void*)&kernel_sem_destroy);
    module.addSymbolExport("cDW233RAwWo", "sem_destroy", "libScePosix", "libkernel", (void*)&kernel_sem_destroy);
    
    module.addSymbolExport("6ULAa0fq4jA", "scePthreadRwlockInit", "libkernel", "libkernel", (void*)&scePthreadRwlockInit);
    module.addSymbolExport("iGjsr1WAtI0", "pthread_rwlock_rdlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_rdlock);
    module.addSymbolExport("Ox9i0c7L5w0", "scePthreadRwlockRdlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_rdlock);
    module.addSymbolExport("sIlRvQqsN2Y", "pthread_rwlock_wrlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_wrlock);
    module.addSymbolExport("mqdNorrB+gI", "scePthreadRwlockWrlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_wrlock);
    module.addSymbolExport("XhWHn6P5R7U", "pthread_rwlock_trywrlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_trywrlock);
    module.addSymbolExport("XhWHn6P5R7U", "pthread_rwlock_trywrlock", "libScePosix", "libkernel", (void*)&kernel_pthread_rwlock_trywrlock);
    module.addSymbolExport("bIHoZCTomsI", "scePthreadRwlockTrywrlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_trywrlock);
    module.addSymbolExport("SFxTMOfuCkE", "pthread_rwlock_tryrdlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_tryrdlock);
    module.addSymbolExport("SFxTMOfuCkE", "pthread_rwlock_tryrdlock", "libScePosix", "libkernel", (void*)&kernel_pthread_rwlock_tryrdlock);
    module.addSymbolExport("XD3mDeybCnk", "scePthreadRwlockTryrdlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_tryrdlock);
    module.addSymbolExport("EgmLo6EWgso", "pthread_rwlock_unlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_unlock);
    module.addSymbolExport("+L98PIbGttk", "scePthreadRwlockUnlock", "libkernel", "libkernel", (void*)&kernel_pthread_rwlock_unlock);
    module.addSymbolExport("yOfGg-I1ZII", "scePthreadRwlockattrInit", "libkernel", "libkernel", (void*)&kernel_pthread_rwlockattr_init);
    
    module.addSymbolExport("B+vc2AO2Zrc", "sceKernelAllocateMainDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateMainDirectMemory);
    module.addSymbolExport("rTXw65xmLIA", "sceKernelAllocateDirectMemory", "libkernel", "libkernel", (void*)&sceKernelAllocateDirectMemory);
    module.addSymbolExport("L-Q3LEjIbgA", "sceKernelMapDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapDirectMemory);
    module.addSymbolExport("NcaWUxfMNIQ", "sceKernelMapNamedDirectMemory", "libkernel", "libkernel", (void*)&sceKernelMapNamedDirectMemory);
    module.addSymbolExport("IWIBBdTHit4", "sceKernelMapFlexibleMemory", "libkernel", "libkernel", (void*)&sceKernelMapFlexibleMemory);
    module.addSymbolExport("mL8NDH86iQI", "sceKernelMapNamedFlexibleMemory", "libkernel", "libkernel", (void*)&sceKernelMapNamedFlexibleMemory);
    module.addSymbolExport("kc+LEEIYakc", "sceKernelMapNamedSystemFlexibleMemory", "libkernel", "libkernel", (void*)&sceKernelMapNamedSystemFlexibleMemory);
    module.addSymbolExport("7oxv3PPCumo", "sceKernelReserveVirtualRange", "libkernel", "libkernel", (void*)&sceKernelReserveVirtualRange);
    module.addSymbolExport("MBuItvba6z8", "sceKernelReleaseDirectMemory", "libkernel", "libkernel", (void*)&sceKernelReleaseDirectMemory);
    module.addSymbolStub("teiItL2boFw", "sceKernelReleaseFlexibleMemory", "libkernel", "libkernel");    // TODO
    module.addSymbolExport("hwVSPCmp5tM", "sceKernelCheckedReleaseDirectMemory", "libkernel", "libkernel", (void*)&sceKernelCheckedReleaseDirectMemory);
    module.addSymbolExport("cQke9UuBQOk", "sceKernelMunmap", "libkernel", "libkernel", (void*)&sceKernelMunmap);
    module.addSymbolExport("UqDGjXA5yUM", "munmap", "libkernel", "libkernel", (void*)&kernel_munmap);
    module.addSymbolExport("UqDGjXA5yUM", "munmap", "libScePosix", "libkernel", (void*)&kernel_munmap);
    module.addSymbolExport("pO96TwzOm5E", "sceKernelGetDirectMemorySize", "libkernel", "libkernel", (void*)&sceKernelGetDirectMemorySize);
    module.addSymbolExport("n1-v6FgU7MQ", "sceKernelConfiguredFlexibleMemorySize", "libkernel", "libkernel", (void*)&sceKernelConfiguredFlexibleMemorySize);
    module.addSymbolExport("BC+OG5m9+bw", "sceKernelGetDirectMemoryType", "libkernel", "libkernel", (void*)&sceKernelGetDirectMemoryType);
    module.addSymbolExport("C0f7TJcbfac", "sceKernelAvailableDirectMemorySize", "libkernel", "libkernel", (void*)&sceKernelAvailableDirectMemorySize);
    module.addSymbolExport("aNz11fnnzi4", "sceKernelAvailableFlexibleMemorySize", "libkernel", "libkernel", (void*)&sceKernelAvailableFlexibleMemorySize);
    module.addSymbolExport("rVjRvHJ0X6c", "sceKernelVirtualQuery", "libkernel", "libkernel", (void*)&sceKernelVirtualQuery);
    module.addSymbolExport("BHouLQzh0X0", "sceKernelDirectMemoryQuery", "libkernel", "libkernel", (void*)&sceKernelDirectMemoryQuery);
    module.addSymbolExport("WFcfL2lzido", "sceKernelQueryMemoryProtection", "libkernel", "libkernel", (void*)&sceKernelQueryMemoryProtection);
    module.addSymbolExport("BPE9s9vQQXo", "mmap", "libkernel", "libkernel", (void*)&kernel_mmap);
    module.addSymbolExport("BPE9s9vQQXo", "mmap", "libScePosix", "libkernel", (void*)&kernel_mmap);
    module.addSymbolExport("PGhQHd-dzv8", "sceKernelMmap", "libkernel", "libkernel", (void*)&sceKernelMmap);
    module.addSymbolExport("2SKEx6bSq-4", "sceKernelBatchMap", "libkernel", "libkernel", (void*)&sceKernelBatchMap);
    module.addSymbolExport("kBJzF8x4SyE", "sceKernelBatchMap2", "libkernel", "libkernel", (void*)&sceKernelBatchMap2);
    module.addSymbolStub("YQOfxL4QfeU", "mprotect", "libkernel", "libkernel");
    module.addSymbolStub("9bfdLIyuwCY", "sceKernelMtypeprotect", "libkernel", "libkernel");
    module.addSymbolStub("Jahsnh4KKkg", "madvise", "libkernel", "libkernel");
    
    module.addSymbolExport("wzvqT4UqKX8", "sceKernelLoadStartModule", "libkernel", "libkernel", (void*)&sceKernelLoadStartModule);
    module.addSymbolExport("Y1nEpkCieOY", "sceKernelLoadStartModuleInternalForMono", "libkernel", "libkernel", (void*)&sceKernelLoadStartModuleInternalForMono);
    module.addSymbolExport("LwG8g3niqwA", "sceKernelDlsym", "libkernel", "libkernel", (void*)&sceKernelDlsym);
    
    module.addSymbolStub("VOx8NGmHXTs", "sceKernelGetCpumode", "libkernel", "libkernel", 5 /* normal 7cpu mode */);
    module.addSymbolStub("VHCS3rCd0PM", "sceKernelAddReadEvent", "libkernel", "libkernel"); // TODO: Important if not used for sockets
    module.addSymbolStub("qBDmpCyGssE", "scePthreadCancel", "libkernel", "libkernel");
    module.addSymbolStub("sCJd99Phct0", "scePthreadSetcanceltype", "libkernel", "libkernel");
    module.addSymbolStub("oVZ+-KgZJGo", "scePthreadSetDefaultstacksize", "libkernel", "libkernel");
    module.addSymbolStub("AUXVxWeJU-A", "sceKernelUnlink", "libkernel", "libkernel");
    module.addSymbolStub("5txKfcMUAok", "pthread_mutexattr_setprotocol", "libkernel", "libkernel");
    module.addSymbolStub("5txKfcMUAok", "pthread_mutexattr_setprotocol", "libScePosix", "libkernel");
    module.addSymbolStub("EXv3ztGqtDM", "pthread_mutexattr_setpshared", "libkernel", "libkernel");
    module.addSymbolStub("EXv3ztGqtDM", "pthread_mutexattr_setpshared", "libScePosix", "libkernel");
    module.addSymbolStub("nTxZBp8YNGc", "pthread_mutex_setname_np", "libkernel", "libkernel");
    module.addSymbolStub("HF7lK46xzjY", "pthread_mutexattr_destroy", "libkernel", "libkernel");
    module.addSymbolStub("HF7lK46xzjY", "pthread_mutexattr_destroy", "libScePosix", "libkernel");
    module.addSymbolStub("3BpP850hBT4", "pthread_condattr_setpshared", "libkernel", "libkernel");
    module.addSymbolStub("3BpP850hBT4", "pthread_condattr_setpshared", "libScePosix", "libkernel");
    module.addSymbolStub("EjllaAqAPZo", "pthread_condattr_setclock", "libkernel", "libkernel");
    module.addSymbolStub("EjllaAqAPZo", "pthread_condattr_setclock", "libScePosix", "libkernel");
    module.addSymbolStub("532IaQguwMg", "scePthreadMutexattrSetprioceiling", "libkernel", "libkernel");
    module.addSymbolStub("i2ifZ3fS2fo", "scePthreadRwlockattrDestroy", "libkernel", "libkernel");
    module.addSymbolStub("BB+kb08Tl9A", "scePthreadRwlockDestroy", "libkernel", "libkernel");
    module.addSymbolStub("euKRgm0Vn2M", "pthread_attr_setschedparam", "libkernel", "libkernel");
    module.addSymbolStub("euKRgm0Vn2M", "pthread_attr_setschedparam", "libScePosix", "libkernel");
    module.addSymbolStub("qlk9pSLsUmM", "pthread_attr_getschedparam", "libkernel", "libkernel");
    module.addSymbolStub("qlk9pSLsUmM", "pthread_attr_getschedparam", "libScePosix", "libkernel");
    module.addSymbolStub("JarMIy8kKEY", "pthread_attr_setschedpolicy", "libkernel", "libkernel");
    module.addSymbolStub("JarMIy8kKEY", "pthread_attr_setschedpolicy", "libScePosix", "libkernel");
    module.addSymbolStub("Xs9hdiD7sAA", "pthread_setschedparam", "libkernel", "libkernel");
    module.addSymbolStub("Xs9hdiD7sAA", "pthread_setschedparam", "libScePosix", "libkernel");
    module.addSymbolStub("FIs3-UQT9sg", "pthread_getschedparam", "libkernel", "libkernel");
    module.addSymbolStub("FIs3-UQT9sg", "pthread_getschedparam", "libScePosix", "libkernel");
    module.addSymbolStub("ZMn3clnAGBA", "pthread_spin_init", "libkernel", "libkernel");
    module.addSymbolStub("ZMn3clnAGBA", "pthread_spin_init", "libScePosix", "libkernel");
    module.addSymbolStub("rCTGkBIHfPY", "pthread_spin_trylock", "libkernel", "libkernel");
    module.addSymbolStub("LEfMMCT+SlM", "pthread_spin_unlock", "libkernel", "libkernel");
    module.addSymbolStub("IJIggoPZExk", "pthread_spin_destroy", "libkernel", "libkernel");
    module.addSymbolStub("oxMp8uPqa+U", "pthread_set_name_np", "libkernel", "libkernel");
    module.addSymbolStub("P41kTWUS3EI", "scePthreadGetschedparam", "libkernel", "libkernel");
    module.addSymbolStub("oIRFTjoILbg", "scePthreadSetschedparam", "libkernel", "libkernel");
    module.addSymbolStub("8mql9OcQnd4", "sceKernelDeleteEventFlag", "libkernel", "libkernel");
    module.addSymbolStub("1FGvU0i9saQ", "scePthreadMutexattrSetprotocol", "libkernel", "libkernel");
    module.addSymbolStub("XAzZo12sbN8", "scePthreadMutexSetprioceiling", "libkernel", "libkernel");
    module.addSymbolStub("tZY4+SZNFhA", "msync", "libkernel", "libkernel");
    module.addSymbolStub("crb5j7mkk1c", "_is_signal_return", "libkernel", "libkernel"); // TODO: Important
    module.addSymbolStub("vSMAm3cxYTY", "sceKernelMprotect", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("jh+8XiK4LeE", "sceKernelIsAddressSanitizerEnabled", "libkernel", "libkernel", false);
    module.addSymbolStub("bnZxYgAFeA0", "sceKernelGetSanitizerNewReplaceExternal", "libkernel", "libkernel");
    module.addSymbolStub("py6L8jiVAN8", "sceKernelGetSanitizerMallocReplaceExternal", "libkernel", "libkernel");
    module.addSymbolStub("bt3CTBKmGyI", "scePthreadSetaffinity", "libkernel", "libkernel");
    module.addSymbolStub("rcrVFJsQWRY", "scePthreadGetaffinity", "libkernel", "libkernel");
    module.addSymbolStub("1tKyG7RlMJo", "scePthreadGetprio", "libkernel", "libkernel"); // TODO: Probably important
    module.addSymbolStub("W0Hpm2X0uPE", "scePthreadSetprio", "libkernel", "libkernel");
    module.addSymbolStub("eXbUSpEaTsA", "scePthreadAttrSetinheritsched", "libkernel", "libkernel");
    module.addSymbolStub("DzES9hQF4f4", "scePthreadAttrSetschedparam", "libkernel", "libkernel");
    module.addSymbolStub("FXPWHNk8Of0", "scePthreadAttrGetschedparam", "libkernel", "libkernel");
    module.addSymbolStub("4+h9EzwKF4I", "scePthreadAttrSetschedpolicy", "libkernel", "libkernel");
    module.addSymbolStub("3qxgM4ezETA", "scePthreadAttrSetaffinity", "libkernel", "libkernel");
    module.addSymbolStub("El+cQ20DynU", "scePthreadAttrSetguardsize", "libkernel", "libkernel");
    module.addSymbolStub("rNhWz+lvOMU", "_sceKernelSetThreadDtors", "libkernel", "libkernel");  // void
    module.addSymbolStub("pB-yGZ2nQ9o", "_sceKernelSetThreadAtexitCount", "libkernel", "libkernel");  // void
    module.addSymbolStub("WhCc1w3EhSI", "_sceKernelSetThreadAtexitReport", "libkernel", "libkernel");  // void
    module.addSymbolStub("Tz4RNUCBbGI", "_sceKernelRtldThreadAtexitIncrement", "libkernel", "libkernel");
    module.addSymbolStub("DGMG3JshrZU", "sceKernelSetVirtualRangeName", "libkernel", "libkernel");
    module.addSymbolStub("PfccT7qURYE", "ioctl", "libkernel", "libkernel");
    module.addSymbolStub("fFxGkxF2bVo", "setsockopt", "libkernel", "libkernel");
    module.addSymbolStub("fFxGkxF2bVo", "setsockopt", "libScePosix", "libkernel");
    module.addSymbolStub("T8fER+tIGgk", "select", "libkernel", "libkernel");
    module.addSymbolStub("T8fER+tIGgk", "select", "libScePosix", "libkernel");
    module.addSymbolStub("3e+4Iv7IJ8U", "accept", "libkernel", "libkernel");
    module.addSymbolStub("3e+4Iv7IJ8U", "accept", "libScePosix", "libkernel");
    module.addSymbolStub("pxnCmagrtao", "listen", "libkernel", "libkernel");
    module.addSymbolStub("pxnCmagrtao", "listen", "libScePosix", "libkernel");
    module.addSymbolStub("TXFFFiNldU8", "getpeername", "libkernel", "libkernel");
    module.addSymbolStub("TXFFFiNldU8", "getpeername", "libScePosix", "libkernel");
    module.addSymbolStub("TUuiYS2kE8s", "shutdown", "libkernel", "libkernel");
    module.addSymbolStub("TUuiYS2kE8s", "shutdown", "libScePosix", "libkernel");
    module.addSymbolStub("5dgOEPsEGqw", "scePthreadBarrierInit", "libkernel", "libkernel");
    module.addSymbolStub("t9vVyTglqHQ", "scePthreadBarrierWait", "libkernel", "libkernel");
    module.addSymbolStub("hHlZQUnlxSM", "getrusage", "libkernel", "libkernel");
    module.addSymbolStub("hHlZQUnlxSM", "getrusage", "libScePosix", "libkernel");
    module.addSymbolStub("6BpEZuDT7YI", "pthread_key_delete", "libkernel", "libkernel");
    module.addSymbolStub("6BpEZuDT7YI", "pthread_key_delete", "libScePosix", "libkernel");
    module.addSymbolStub("4oXYe9Xmk0Q", "sceKernelGetGPI", "libkernel", "libkernel");
    module.addSymbolStub("ca7v6Cxulzs", "sceKernelSetGPO", "libkernel", "libkernel");
    module.addSymbolStub("igMefp4SAv0", "get_authinfo", "libkernel", "libkernel");
    module.addSymbolStub("wdUufa9g-D8", "dup2", "libkernel", "libkernel");
    module.addSymbolStub("bt0POEUZddE", "sceKernelGetSanitizerMallocReplace", "libkernel", "libkernel");
    module.addSymbolStub("F4Kib3Mb0wI", "sceKernelGetSanitizerNewReplace", "libkernel", "libkernel");
    module.addSymbolStub("-YTW+qXc3CQ", "sceKernelInternalMemoryGetModuleSegmentInfo", "libkernel", "libkernel");
    module.addSymbolStub("C2ltEJILIGE", "sceKernelGetPsmIntdevModeForRcmgr", "libkernel", "libkernel");
    module.addSymbolStub("ucFJiTO1EUw", "dlerror", "libkernel", "libkernel");
    module.addSymbolStub("8vE6Z6VEYyk", "access", "libkernel", "libkernel");
    module.addSymbolStub("DFmMT80xcNI", "sysctl", "libkernel", "libkernel");
    module.addSymbolStub("fgIsQ10xYVA", "sceKernelChmod", "libkernel", "libkernel");
    module.addSymbolStub("Wl2o5hOVZdw", "sceKernelPrintBacktraceWithModuleInfo", "libkernel", "libkernel");
    module.addSymbolStub("8nY19bKoiZk", "fcntl", "libkernel", "libkernel");
    module.addSymbolStub("mkawd0NA9ts", "sysconf", "libkernel", "libkernel");
    module.addSymbolStub("cfjAjVTFG6A", "pthread_suspend_user_context_np", "libkernel", "libkernel");
    module.addSymbolStub("YkGOXpJEtO8", "pthread_get_user_context_np", "libkernel", "libkernel");
    module.addSymbolStub("QRdE7dBfNks", "pthread_resume_user_context_np", "libkernel", "libkernel");
    module.addSymbolStub("6jj29MbyzuI", "sysKernelGetManufacturingMode", "libkernel", "libkernel");
    module.addSymbolStub("ul57hvm6mBc", "sceKernelGetOpenPsIdForSystem", "libkernel", "libkernel");
    module.addSymbolStub("2YsHtbvCrgs", "sceKernelGetIdTableCurrentCount", "libkernel", "libkernel");
    module.addSymbolStub("mpbGISNJ6go", "sceKernelGetSystemExVersion", "libkernel", "libkernel");
    module.addSymbolStub("B1K98ubk6V8", "sceKernelIsExperimentalBeta", "libkernel", "libkernel");
    module.addSymbolStub("8aCOCGoRkUI", "sceKernelIsCEX", "libkernel", "libkernel");
    module.addSymbolStub("3EDFoWECKOg", "sceKernelGetSystemSwBeta", "libkernel", "libkernel");
    module.addSymbolStub("7p7kTAJcuGg", "__inet_addr", "libkernel", "libkernel");
    module.addSymbolStub("a7ToDPsIQrc", "__inet_aton", "libkernel", "libkernel");
    module.addSymbolStub("6i5aLrxRhG0", "__inet_ntoa", "libkernel", "libkernel");
    module.addSymbolStub("H2QD+kNpa+U", "__inet_ntoa_r", "libkernel", "libkernel");
    module.addSymbolStub("4pYihoPggn8", "__inet_ntop", "libkernel", "libkernel");
    module.addSymbolStub("fyPeCKJ94Hg", "__inet_pton", "libkernel", "libkernel");
    module.addSymbolStub("mTBZfEal2Bw", "mlock", "libkernel", "libkernel");
    module.addSymbolStub("OG4RsDwLguo", "munlock", "libkernel", "libkernel");
    module.addSymbolStub("iBQ2omlTuls", "sceKernelIccSetBuzzer", "libkernel", "libkernel");
    module.addSymbolStub("txHtngJ+eyc", "scePthreadAttrGetguardsize", "libkernel", "libkernel");    // Used by Worms WMD
    
    module.addSymbolExport("KiJEPEWRyUY", "sigaction", "libkernel", "libkernel", (void*)&kernel_sigaction);
    module.addSymbolExport("aPcyptbOiZs", "sigprocmask", "libkernel", "libkernel", (void*)&kernel_sigprocmask);
    module.addSymbolExport("6xVpy0Fdq+I", "_sigprocmask", "libkernel", "libkernel", (void*)&kernel_sigprocmask);
    module.addSymbolStub("+F7C-hdk7+E", "sigemptyset", "libkernel", "libkernel");
    module.addSymbolStub("JUimFtKe0Kc", "sigaddset", "libkernel", "libkernel");
    module.addSymbolStub("Nd-u09VFSCA", "sigdelset", "libkernel", "libkernel");
    module.addSymbolStub("+F7C-hdk7+E", "sigemptyset", "libkernel", "libkernel");
    module.addSymbolStub("VADc3MNQ3cM", "signal", "libkernel", "libkernel");
    
    module.addSymbolExport("QuJYZ2KVGGQ", "shm_open", "libkernel", "libkernel", (void*)&kernel_shm_open);
    module.addSymbolStub("tPWsbOUGO8k", "shm_unlink", "libkernel", "libkernel");
    module.addSymbolStub("n371J5cP+uo", "physhm_open", "libkernel", "libkernel", 10);
    module.addSymbolStub("AUqJNkobQ1c", "physhm_unlink", "libkernel", "libkernel");
    
    module.addSymbolExport("Hk7iHmGxB18", "ipmimgr_call", "libkernel", "libkernel", (void*)&ipmimgr_call);
    
    module.addSymbolStub("mpxAdqW7dKY", "sceKernelIsProspero", "libkernel_cpumode_platform", "libkernel", false);
    
    module.addSymbolStub("3k6kx-zOOSQ", "sceKernelMlock", "libkernel", "libkernel");
    module.addSymbolStub("EfqmKkirJF0", "sceKernelMlockall", "libkernel", "libkernel");
    
    module.addSymbolStub("+YX0z-GUSNw", "sceCoredumpAttachMemoryRegion", "libSceCoredump", "libkernel");
    
    module.addSymbolStub("UtO0OHMCgmI", "sceKernelIsDevelopmentMode", "libSceDipsw", "libSceDipsw");
    
    module.addSymbolStub("XFYItOxS6r0", "sceApplicationInitialize", "libSceSysCore", "libSceSysCore");
    module.addSymbolStub("qTHiabfEukw", "sceApplicationSetCanvasHandle", "libSceSysCore", "libSceSysCore");

    // libSceLibcInternal HLE. Move these to their own file later
    //module.addSymbolExport("gQX+4GDQjpM", "malloc", "libSceLibcInternal", "libSceLibcInternal", (void*)&Kernel::malloc);
    //module.addSymbolExport("tIhsqj0qsFE", "free", "libSceLibcInternal", "libSceLibcInternal", (void*)&Kernel::free);

    proc_counter_start = SDL_GetPerformanceCounter();

    std::memset(environment, 0, sizeof(environment));
    environment[0] = "MONO_GC_PARAMS=nursery-size=1024m,max-heap-size=4096m";
    //environment[1] = "MONO_LOG_LEVEL=debug";
    //environment[2] = "MONO_LOG_MASK=all";
    //environment[3] = "MONO_DISABLE_SHM=1";
    kernel_environ = environment;

    stdout_file.open("stdout.txt", std::ios::binary | std::ios::trunc);
}

static thread_local s32 posix_errno = 0;

#ifdef _WIN32
std::mutex allocator_mtx;

static constexpr uptr SYSTEM_MAPPING_AREA = 0x0010'0000'0000;
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
    // Align up
    cur_addr = (cur_addr + alignment - 1) & ~(alignment - 1);

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

// Same as allocate, without committing physical memory.
void* findNextFree(uptr reservation_start, uptr reservation_end, size_t size, size_t alignment) {
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
    // Align up
    cur_addr = (cur_addr + alignment - 1) & ~(alignment - 1);

    while (true) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery((void*)cur_addr, &mbi, sizeof(mbi)))
            Helpers::panic("allocate: VirtualQuery failed\n");

        if (mbi.State == MEM_RESERVE) {
            uptr region_end = (uptr)mbi.BaseAddress + mbi.RegionSize;
            if (cur_addr + size <= region_end) {
                return (void*)cur_addr;
            }

            // Free area wasn't big enough
        }
        cur_addr = (uptr)mbi.BaseAddress + mbi.RegionSize;
        // Align up
        cur_addr = (cur_addr + alignment - 1) & ~(alignment - 1);

        if (cur_addr > reservation_end) Helpers::panic("allocate: out of memory\n");
    }


    return nullptr;
}

// Same as findNextFree, except it looks for mapped memory
void* findNextMapped(uptr reservation_start, uptr reservation_end, size_t size, size_t alignment) {
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
    // Align up
    cur_addr = (cur_addr + alignment - 1) & ~(alignment - 1);

    while (true) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery((void*)cur_addr, &mbi, sizeof(mbi)))
            Helpers::panic("allocate: VirtualQuery failed\n");

        if (mbi.State == MEM_COMMIT) {
            uptr region_end = (uptr)mbi.BaseAddress + mbi.RegionSize;
            if (cur_addr + size <= region_end) {
                return (void*)cur_addr;
            }

            // Free area wasn't big enough
        }
        cur_addr = (uptr)mbi.BaseAddress + mbi.RegionSize;
        // Align up
        cur_addr = (cur_addr + alignment - 1) & ~(alignment - 1);

        if (cur_addr > reservation_end) return nullptr;
    }


    return nullptr;
}
#endif

s32* PS4_FUNC kernel_error() {
    return &posix_errno;
}

s32 PS4_FUNC kernel_getpagesize() {
    return 16_KB;
}

void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx) {
    log("__tls_get_addr(tls_idx=*%p)\n", tls_idx);
    //log("modid=%d, offset=0x%x\n", tls_idx->modid, tls_idx->offset);
    return (void*)((u64)Thread::getTLSPtr(tls_idx->modid) + tls_idx->offset);
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

s32 PS4_FUNC sceKernelNanosleep(const SceKernelTimespec* rqtp, SceKernelTimespec* rmtp) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(rqtp->tv_nsec) + std::chrono::seconds(rqtp->tv_sec));
    return SCE_OK;
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
    case SCE_KERNEL_CLOCK_EXT_NETWORK:
    case SCE_KERNEL_CLOCK_EXT_AD_NETWORK:
    case SCE_KERNEL_CLOCK_EXT_RAW_NETWORK:
    case SCE_KERNEL_CLOCK_MONOTONIC_PRECISE:
    case SCE_KERNEL_CLOCK_MONOTONIC_FAST:
    case SCE_KERNEL_CLOCK_MONOTONIC: {
        auto counter = SDL_GetPerformanceCounter();
        auto freq = SDL_GetPerformanceFrequency();
        ts->tv_sec = counter / freq;
        ts->tv_nsec = (counter % freq) * 1000000000ull / freq;
        break;
    }

    case SCE_KERNEL_CLOCK_REALTIME: // TODO: I don't think this is correct
    case SCE_KERNEL_CLOCK_SECOND: {
        const auto now = std::chrono::system_clock::now();
        const auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);

        ts->tv_sec = sec.time_since_epoch().count();
        ts->tv_nsec = clock_id != SCE_KERNEL_CLOCK_SECOND ? ns.count() : 0;
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
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - sec);

    if (tv) {
        tv->tv_sec = sec.time_since_epoch().count();
        tv->tv_usec = us.count();
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

s32 PS4_FUNC sceKernelConvertUtcToLocaltime(time_t time, time_t* local_time, SceKernelTimesec* st, u64* dst_sec) {
    log("sceKernelConvertUtcToLocaltime(time=%lld, local_time=*%p, st=*%p, dst_sec=*%p)\n", time, local_time, st, dst_sec);

    // TODO
    *local_time = time;
    st->t = time;
    st->west_sec = 0;
    st->dst_sec = 0;
    if (dst_sec) *dst_sec = 0;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelConvertLocaltimeToUtc(time_t local_time, s64 unk1, time_t* time, SceKernelTimezone* timezone, u64* dst_sec) {
    log("sceKernelConvertLocaltimeToUtc(local_time=%lld, unk1=%lld, time=*%p, timezone=*%p, dst_sec=*%p)\n", local_time, unk1, time, timezone, dst_sec);

    // TODO
    if (time) *time = local_time;
    if (dst_sec) *dst_sec = 0;
    return SCE_OK;
}

static const auto process_start_time = std::chrono::steady_clock::now();
u64 PS4_FUNC sceKernelGetProcessTime() {
    //log("sceKernelGetProcessTime()\n");
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - process_start_time).count();
}

u64 PS4_FUNC sceKernelGetProcessTimeCounter() {
    //log("sceKernelGetProcessTimeCounter()\n");
    const auto now = SDL_GetPerformanceCounter();
    return now - proc_counter_start;
}

u64 PS4_FUNC sceKernelGetProcessTimeCounterFrequency() {
    //log("sceKernelGetProcessTimeCounterFrequency()\n");
    return SDL_GetPerformanceFrequency();
}

void PS4_FUNC kernel_sched_yield() {
    //std::this_thread::yield();
}

u64 PS4_FUNC sceKernelReadTsc() {
    log("sceKernelReadTsc()\n");
    return __rdtsc();
}

s32 PS4_FUNC sceKernelIsNeoMode() {
    log("sceKernelIsNeoMode()\n");
    return false;
}

void* PS4_FUNC sceKernelGetProcParam() {
    log("sceKernelGetProcParam()\n");
    return (void*)g_app.modules[0]->proc_param_ptr;
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
                //printf("Measured frequency: %lld\n", tsc_freq);
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

s32 PS4_FUNC sceKernelGetAppInfo(s32 pid, SceKernelAppInfo* app_info) {
    log("sceKernelGetAppInfo(pid=%d, app_info=*%p)\n", pid, app_info);

    // We assume pid is the current process and we only return the title ID
    std::memset(app_info, 0, sizeof(SceKernelAppInfo));
    app_info->has_param_sfo = true;
    std::strncpy(app_info->cusa_name, g_app.title_id.c_str(), 10);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelTitleWorkaroundIsEnabled(SceKernelTitleWorkaround* workaround, s32 bit, s32* result) {
    log("sceKernelTitleWorkaroundIsEnabled(workaround=*%p, bit=%d, result=*%p)\n", workaround, bit, result);
    *result = 0;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetSystemSwVersion(SceKernelSwVersion* ver) {
    log("sceKernelGetSystemSwVersion(ver=*%p)\n", ver);

    ver->hex = 0x11500001;
    std::strcpy(ver->text, "11.500.001");
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetModuleInfo2(s32 handle, SceKernelModuleInfo* info) {
    log("sceKernelGetModuleInfo2(handle=%d, info=*%p)\n", handle, info);

    auto mod = g_app.findModule(handle);
    if (!mod) Helpers::panic("sceKernelGetModuleInfo2: no module with handle %d\n", handle);

    printf("module is %s\n", mod->filename.c_str());
    std::strncpy(info->name, mod->filename.c_str(), SCE_DBG_MAX_NAME_LENGTH);   // TODO: I don't think the filename is the correct name
    std::memset(info->segments, 0, sizeof(SceKernelModuleSegmentInfo) * SCE_DBG_MAX_SEGMENTS);
    for (int i = 0; i < mod->n_segments; i++) {
        info->segments[i].addr = mod->segments[i].addr;
        info->segments[i].size = mod->segments[i].size;
        info->segments[i].prot = mod->segments[i].prot;
    }
    info->segment_count = mod->n_segments;
    std::memset(info->fingerprint, 0, sizeof(u8) * SCE_DBG_NUM_FINGERPRINT);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetModuleList2(s32* handles, u64 n_handles, u64* out_n_handles) {
    log("sceKernelGetModuleList2(handles=%p, n_handles=%lld, out_n_handles=*%p)\n", handles, n_handles, out_n_handles);

    u64 cnt = 0;
    g_app.forEachModule([&](auto mod) -> bool {
        if (mod->filename == "HLE") return false;

        handles[cnt++] = mod->modid;
        if (cnt >= n_handles) return true;
        else return false;
    });

    *out_n_handles = cnt;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetModuleInfoFromAddr(void* addr, s32 flags, SceKernelModuleInfoEx* info) {
    log("sceKernelGetModuleInfoFromAddr(addr=%p, flags=%d, info=*%p)\n", addr, flags, info);

    auto mod = g_app.findModuleByAddress(addr);
    if (!mod)
        return SCE_KERNEL_ERROR_ESRCH;

    std::strncpy(info->name, mod->filename.c_str(), SCE_DBG_MAX_NAME_LENGTH);   // TODO: I don't think the filename is the correct name
    info->id                = mod->modid;
    info->tls_index         = mod->tls_modid;
    info->tls_init_addr     = (void*)mod->tls_vaddr;
    info->tls_size          = mod->tls_filesz;
    info->tls_offset        = 0;    // ?
    info->tls_align         = 1;    // ?
    info->init_proc_addr    = (void*)mod->init_func;
    info->fini_proc_addr    = nullptr;  // TODO
    info->eh_frame_hdr_addr = mod->eh_frame_hdr_addr;
    info->eh_frame_addr     = nullptr;  // TODO
    info->eh_frame_hdr_size = mod->eh_frame_hdr_size;
    info->eh_frame_size     = 0;        // TODO
    std::memset(info->segments, 0, sizeof(SceKernelModuleSegmentInfo) * SCE_DBG_MAX_SEGMENTS);
    for (int i = 0; i < mod->n_segments; i++) {
        info->segments[i].addr = mod->segments[i].addr;
        info->segments[i].size = mod->segments[i].size;
        info->segments[i].prot = mod->segments[i].prot;
    }
    info->segment_count = mod->n_segments;

    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetModuleInfoForUnwind(void* addr, s32 flags, SceKernelModuleInfoForUnwind* info) {
    log("sceKernelGetModuleInfoForUnwind(addr=%p, flags=%d, info=*%p)\n", addr, flags, info);

    auto mod = g_app.findModuleByAddress(addr);

    info->seg0_addr = mod->segments[0].addr;
    info->seg0_size = mod->segments[0].size;
    return -1;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelDebugRaiseExceptionOnReleaseMode(u32 error) {
    log("sceKernelDebugRaiseExceptionOnReleaseMode(error=%d)\n", error);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    stdout_file.flush();
    Helpers::panic("Exception raised: %d\n", error);
    return SCE_OK;
}

s32 PS4_FUNC kernel_getrlimit(s32 resource, kernel_rlimit* rlim) {
    log("getrlimit(resource=%d, rlim=*%p)\n", resource, rlim);

    switch (resource) {
    case KERNEL_RLIMIT_DATA: {
        // ?
        rlim->rlim_cur = 0xffffffffffffffff;
        rlim->rlim_max = 0xffffffffffffffff;
        break;
    }

    case KERNEL_RLIMIT_STACK: {
        // ?
        rlim->rlim_cur = 0xffffffffffffffff;
        rlim->rlim_max = 0xffffffffffffffff;
        break;
    }

    case KERNEL_RLIMIT_NOFILE: {
        rlim->rlim_cur = 256;
        rlim->rlim_max = 256;
        break;
    }

    default: Helpers::panic("getrlimit: unhandled resource %d\n", resource);
    }

    return SCE_OK;
}

s64 mono_mspace = 0;
s64 PS4_FUNC sceLibcMspaceCreateForMonoMutex(s64 unk1, s32 unk2, s32 unk3, s64 unk4) {
    log("sceLibcMspaceCreateForMonoMutex(unk1=%lld, unk2=%d, unk3=%d, unk4=%lld)\n", unk1, unk2, unk3, unk4);

    if (mono_mspace) {
        Helpers::panic("sceLibcMspaceCreateForMonoMutex: called twice\n");
    }

    if (unk1 || unk2 || unk3 || unk4) {
        Helpers::panic("sceLibcMspaceCreateForMonoMutex: invalid parameters\n");
    }

    void* addr = nullptr;
    auto err = sceKernelMapNamedSystemFlexibleMemory(&addr, 10_MB, 3, 0, "MonoMtx");
    if (err) {
        Helpers::panic("sceLibcMspaceCreateForMonoMutex: failed to map memory\n");
    }

    if (!addr) {
        Helpers::panic("sceLibcMspaceCreateForMonoMutex: out_addr is nullptr\n");
    }

    auto mod = g_app.findModuleByName("libSceLibcInternal.sprx");
    auto* sym = mod->findSymbolExport(Helpers::nameToNid("sceLibcMspaceCreate"));
    mono_mspace = ((PS4_FUNC s64(*)(const char*, void*, size_t, s64))(sym->ptr))("MonoMtx", addr, 10_MB, 12);

    sym = mod->findSymbolExport("wUqJ0psUjDo"); // No idea what this is
    ((PS4_FUNC s64(*)(s64))(sym->ptr))(mono_mspace);

    return mono_mspace;
}

s32 PS4_FUNC sceKernelGetCompiledSdkVersion(s32* ver) {
    log("sceKernelGetCompiledSdkVersion(ver=*%p)\n", ver);
    // TODO: Get from ELF header
    *ver = 0x5500000;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetProcessName(s64 pid, char* name) {
    log("sceKernelGetProcessName(pid=%lld, name=*%p)\n", pid, name);

    // TODO: Don't know what the max length is
    const auto name_str = std::format("proc_{}", pid);
    std::strncpy(name, name_str.c_str(), 16);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetDataTransferMode(s32* mode) {
    log("sceKernelGetDataTransferMode(mode=*%p)\n", mode);

    *mode = 0;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetBackupRestoreMode(s32* mode) {
    log("sceKernelGetBackupRestoreMode(mode=*%p)\n", mode);

    *mode = 0;
    return SCE_OK;
}

s32 PS4_FUNC kernel_getargc() {
    return g_app.params.argc;
}

const char** PS4_FUNC kernel_getargv() {
    return g_app.params.argv;
}

s32 PS4_FUNC kernel_getpid() {
    log("getpid()\n");
    return 100;
}

s32 PS4_FUNC kernel_sched_get_priority_max() {
    log("sched_get_priority_max()\n");
    return 767;
}

s32 PS4_FUNC kernel_sched_get_priority_min() {
    log("sched_get_priority_min()\n");
    return 256;
}

s32 PS4_FUNC sigfillset() {
    // TODO
    unimpl("sigfillset() TODO\n");
    return SCE_OK;
}

void PS4_FUNC sceKernelDebugOutText(s64 unknown, char* text) {
    std::printf(text);
    stdout_file.write(text, std::strlen(text));
}

s32 PS4_FUNC __sys_regmgr_call() {
    // TODO
    unimpl("__sys_regmgr_call() TODO\n");
    return SCE_OK;
}

s32 PS4_FUNC kernel_sigaction(s32 sig, Sigaction* act, Sigaction* oact) {
    log("sigaction(sig=%d, act=*%p, oact=*%p) TODO\n", sig, act, oact);
    // TODO
    if (oact)
        std::memset(oact, 0, sizeof(Sigaction));
    return SCE_OK;
}

s32 PS4_FUNC kernel_sigprocmask() {
    log("_sigprocmask() TODO\n");
    return 0;
}

s32 shm_idx = 1;
s32 PS4_FUNC kernel_shm_open(const char* path, s32 flags, s32 /* mode_t */ mode) {
    log("shm_open(path=\"%s\", flags=%d, mode=%d)\n", path, flags, mode);

    return shm_idx++;
}

s32 PS4_FUNC ipmimgr_call(s64 cmd, s64 unk2, u32* res, u8* args, size_t arg_size_bytes, u64 unk3) {
    log("ipmimgr_call(cmd=0x%llx, unk2=%lld, res=*%p, args=*%p, arg_size_bytes=%lld, unk3=0x%llx)\n", cmd, unk2, res, args, arg_size_bytes, unk3);
    
    switch (cmd) {
    // Create server
    case 0x0: {
        *res = 0;
        break;
    }

    // Create client
    case 0x2: {
        std::string name = *(const char**)(args + sizeof(uptr));
        log("Create client \"%s\"", name.c_str());
        *res = -1;
        PWSTR thread_name;
        GetThreadDescription(GetCurrentThread(), &thread_name);
        
        if (name == "SceVnaIpcServer") {
            while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (    name == "SceMorpheusUpdService"
            ||  name == "SceCompAppProxyUtil"
            ||  name == "SceCompAppProxy"
            ||  name == "SceShellAppProxy"
            ||  name == "SceStickerCoreServer"
            ||  name == "SceNpPartyIpc"
            ||  name == "ScePartyIpcService"
            ||  name == "SceAppDbIpc"
           )
            *res = 0;

        if (name == "ScePartyIpcService") {
            while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        break;
    }

    // Destroy client
    case 0x3: {
        *res = 0;
        break;
    }

    // Don't know
    case 0x10: {
        *res = 0;
        break;
    }

    // Receive packet
    case 0x201: {
        while (true) std::this_thread::sleep_for(std::chrono::seconds(1));

        *res = 0;
        break;
    }

    // Try get message
    case 0x252: {
        *res = SCE_KERNEL_ERROR_EAGAIN;
        break;
    }

    // Disconnect
    case 0x310: {
        *res = 0;
        break;
    }

    case 0x320: {
        *res = 0;
        break;
    }

    // Connect
    case 0x400: {
        *res = 0;
        break;
    }
    }

    return SCE_OK;
}

// TODO: Properly implement the virtual memory map

std::unordered_map<void*, u64> virt_dmem_map;
std::unordered_map<u64, void*> dmem_virt_map;
std::map<u64, size_t> dmem_size_map;

u64 next_dmem_addr = 0x10000;
s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr) {
    log("sceKernelAllocateMainDirectMemory(size=0x%016llx, align=0x%016llx, mem_type=%d, out_addr=*%p)\n", size, align, mem_type, out_addr);
    
    // TODO: For now we allocate memory directly in the map function
    //       Eventually I will need to handle the physical memory map properly...
    *out_addr = (void*)next_dmem_addr;
    next_dmem_addr += size;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr) {
    log("sceKernelAllocateDirectMemory(search_start=%p, search_end=%p, size=0x%016llx, align=0x%016llx, mem_type=%d, out_addr=*%p)\n", search_start, search_end, size, align, mem_type, out_addr);

    // TODO: For now we allocate memory directly in the map function
    //       Eventually I will need to handle the physical memory map properly...
    *out_addr = (void*)next_dmem_addr;
    next_dmem_addr += size; 
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align) {
    log("sceKernelMapDirectMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, dmem_start=0x%016llx, align=0x%016llx)\n", addr, len, prot, flags, dmem_start, align);

    void* in_addr = *addr;
    log("in_addr=%p\n", in_addr);

    align = align ? align : 16_KB;

    // Address is out of bounds. FIFA 14 does this, and expects it to work?
    // This game doesn't request a fixed mapping so we can just treat it as in_addr == 0
    if (in_addr > (void*)0xff'ffff'ffff) {
        printf("sceKernelMapDirectMemory: in_addr is out of bounds\n");
        //return SCE_KERNEL_ERROR_ENOMEM;
        in_addr = nullptr;
    }

    // TODO: prot, flags, verify align is a valid value (multiple of 16kb)
#ifdef _WIN32
    if (!in_addr) {
        *addr = allocate(0x8000'0000, 0x8000'0000 + 2000_GB, len, align);
    }
    else if ((u64)in_addr >= 0x8000'0000)
        *addr = allocate((u64)in_addr, 0x8000'0000 + 2000_GB, len, align);
    else
        // TODO
        *addr = allocate(0x8000'0000, 0x8000'0000 + 2000_GB, len, align);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    if (!*addr) {
        Helpers::panic("sceKernelMapDirectMemory: failed to allocate\n");
    }

    if ((flags & SCE_KERNEL_MAP_FIXED) && *addr != in_addr) {
        if (virt_dmem_map.contains(in_addr)) {
            printf("dmem %p was mapped at in_addr with size %d. mapping requested size %d\n", virt_dmem_map[in_addr], dmem_size_map[virt_dmem_map[in_addr]], len);
        }
#ifdef _WIN32
        uptr curr_addr = (uptr)in_addr;
        while (curr_addr < (uptr)in_addr + len) {
            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery((void*)curr_addr, &mbi, sizeof(mbi));

            printf("Memory was reserved from % p to % p with state 0x%x\n", mbi.BaseAddress, (uptr)mbi.BaseAddress + mbi.RegionSize, mbi.State);
            curr_addr = (uptr)mbi.BaseAddress + mbi.RegionSize;
        }
#endif
        Helpers::panic("sceKernelMapDirectMemory: could not allocate at in_addr with fixed flag (got addr %p, requested %p)\n", *addr, in_addr);
        //printf("sceKernelMapDirectMemory: could not allocate at in_addr with fixed flag (got addr %p, requested %p)\n", *addr, in_addr);
        sceKernelMunmap(*addr, len);
        *addr = nullptr;
        return SCE_KERNEL_ERROR_ENOMEM;
    }

    virt_dmem_map[*addr] = (u64)dmem_start;
    dmem_virt_map[(u64)dmem_start] = *addr;
    dmem_size_map[(u64)dmem_start] = len;

    // Clear allocated memory
    std::memset(*addr, 0, len);

    log("Allocated at %p (dmem handle=0x%llx)\n", *addr, dmem_start);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapNamedDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align, const char* name) {
    log("sceKernelMapNamedDirectMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, dmem_start=0x%016llx, align=0x%016llx, name=\"%s\")\n", addr, len, prot, flags, dmem_start, align, name);
    return sceKernelMapDirectMemory(addr, len, prot, flags, dmem_start, align);
}

s32 PS4_FUNC sceKernelMapFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags) {
    log("sceKernelMapFlexibleMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d)\n", addr, len, prot, flags);
    return sceKernelMapNamedFlexibleMemory(addr, len, prot, flags, "unnamed");
}

s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name) {
    log("sceKernelMapNamedFlexibleMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, name=\"%s\")\n", addr, len, prot, flags, name);

    void* in_addr = *addr;
    if (in_addr) {
        Helpers::panic("TODO: sceKernelMapNamedFlexibleMemory with non-zero input addr\n");
    }

    // TODO: prot, flags
#ifdef _WIN32
    *addr = allocate(0x8000'0000, 0x8000'0000 + 2000_GB, len, 16_KB);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    // Clear allocated memory
    std::memset(*addr, 0, len);

    log("Allocated at %p\n", *addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelMapNamedSystemFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name) {
    log("sceKernelMapNamedSystemFlexibleMemory(addr=*%p, len=0x%llx, prot=%d, flags=%d, name=\"%s\") [forwarding to sceKernelMapNamedFlexibleMemory]\n", addr, len, prot, flags, name);
    return sceKernelMapNamedFlexibleMemory(addr, len, prot, flags, name);
}

struct ReservedArea {
    uptr start = 0;
    size_t size = 0;
};
std::vector<ReservedArea> reserved_areas;

s32 PS4_FUNC sceKernelReserveVirtualRange(void** addr, size_t len, s32 flags, size_t align) {
    log("sceKernelReserveVirtualRange(addr=*%p, len=0x%llx, flags=%d, align=0x%016llx)\n", addr, len, flags, align);
    log("in_addr=%p\n", *addr);

    // Quick hack: if MAP_FIXED is specified and MAP_NO_OVERWRITE isn't, just reserve the input address directly
    if ((flags & SCE_KERNEL_MAP_FIXED) && !(flags & 0x80)) {
        if (!*addr) {
            Helpers::panic("sceKernelReserveVirtualRange: MAP_FIXED was specified but *addr is null\n");
        }

        // Leave *addr unchanged
        
        reserved_areas.push_back({ .start = (uptr)*addr, .size = len });
        log("out_addr=%p [skipped]\n", *addr);
        return SCE_OK;
    }

    // Search forwards from in_addr
    void* in_addr = *addr;
    void* original_addr = in_addr;
    void* out_addr = nullptr;
    while (true) {
        // If the address we got was already reserved continue searching
        if ((u64)in_addr >= 0x8000'0000)
            out_addr = findNextFree((u64)in_addr, 0x8000'0000 + 2000_GB, len, align);
        else
            // TODO                   
            out_addr = findNextFree(SYSTEM_MAPPING_AREA, SYSTEM_MAPPING_AREA + 2000_GB, len, align);
    
        bool overlap = false;
        for (auto& area : reserved_areas) {
            if (   Helpers::inRangeSized<uptr>((uptr)out_addr, area.start, area.size)
                || Helpers::inRangeSized<uptr>((uptr)area.start, (uptr)out_addr, len)
               ) {
                overlap = true;
                break;
            }
        }

        if (overlap) {
            in_addr = (void*)((uptr)out_addr + len);
            continue;
        }

        break;
    }

    // Check for fixed mapping and error only if the NO_OVERWRITE flag is specified
    if ((flags & SCE_KERNEL_MAP_FIXED) && out_addr != original_addr) {
        if (flags & 0x80) {     // SCE_KERNEL_MAP_NO_OVERWRITE
#ifdef _WIN32
            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery(in_addr, &mbi, sizeof(mbi));

            log("Could not reserve area at %p. Memory was reserved from %p to %p with state 0x%x\n", in_addr, mbi.BaseAddress, (uptr)mbi.BaseAddress + mbi.RegionSize, mbi.State);
#endif
            Helpers::panic("sceKernelReserveVirtualRange: could not allocate memory at fixed mapping (in_addr=%p, out_addr=%p)\n", in_addr, out_addr);
        }
        else {
            // Just use the input address since we can overwrite it
            out_addr = original_addr;
        }
    }

    *addr = out_addr;
    reserved_areas.push_back({ .start = (uptr)out_addr, .size = len });

    log("out_addr=%p\n", out_addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelReleaseDirectMemory(void* addr, size_t len) {
    log("sceKernelReleaseDirectMemory(addr=%p, len=0x%llx)\n", addr, len);

    // TODO: Implement properly
    //auto to_free = len;
    //while (to_free) {
    //    if (dmem_virt_map.contains((u64)addr)) {
    //        log("releasing memory\n");
    //        void* virt_addr = dmem_virt_map[(u64)addr];
    //        const auto size = std::min(dmem_size_map[(u64)addr], to_free);
    //        sceKernelMunmap(virt_addr, size);
    //        virt_dmem_map.erase(virt_addr);
    //        dmem_virt_map.erase((u64)addr);
    //        dmem_size_map.erase((u64)addr);
    //        addr = (void*)((u64)addr + 1);
    //        to_free -= size;
    //    }
    //    else break;
    //}
    if (dmem_virt_map.contains((u64)addr)) {
        log("releasing memory\n");
        void* virt_addr = dmem_virt_map[(u64)addr];
        sceKernelMunmap(virt_addr, len);
        virt_dmem_map.erase(virt_addr);
        dmem_virt_map.erase((u64)addr);

        auto size = dmem_size_map[(u64)addr];
        dmem_size_map.erase((u64)addr);
        
        if (len < size) {
            const u64 new_virt_addr = (u64)virt_addr + len;
            const u64 new_dmem_addr = (u64)addr + len;
            const u64 new_size = size - len;
            virt_dmem_map[(void*)new_virt_addr] = new_dmem_addr;
            dmem_virt_map[new_dmem_addr] = (void*)new_virt_addr;
            dmem_size_map[new_dmem_addr] = new_size;
        }
            
    }
    else
        printf("sceKernelReleaseDirectMemory: no match for %p\n", addr);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelCheckedReleaseDirectMemory(void* addr, size_t len) {
    log("sceKernelCheckedReleaseDirectMemory(addr=%p, len=0x%llx)\n", addr, len);

    //if (!addr) return SCE_KERNEL_ERROR_ENOENT;

    // TODO: Check if an unallocated area is included
    return sceKernelReleaseDirectMemory(addr, len);
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

s32 PS4_FUNC kernel_munmap(void* addr, size_t len) {
    log("munmap(addr=%p, len=0x%llx)\n", addr, len);
    
    // TODO: Errors
    sceKernelMunmap(addr, len);
    return SCE_OK;
}

size_t PS4_FUNC sceKernelGetDirectMemorySize() {
    log("sceKernelGetDirectMemorySize()\n");
    //return 5_GB;    // Stub for now, we need to get the flexible memory size from the SELF
    return 5_GB - 512_MB;   // total size - flexible mem size
}

s32 PS4_FUNC sceKernelConfiguredFlexibleMemorySize(size_t* out_size) {
    log("sceKernelConfiguredFlexibleMemorySize(out_size=*%p)\n", out_size);
    
    // TODO
    *out_size = 1_GB;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelGetDirectMemoryType(void* start, s32* out_type, void** out_region_start, void** out_region_end) {
    log("sceKernelGetDirectMemoryType(start=%p, out_type=*%p, out_region_start=*%p, out_region_end=*%p)\n", start, out_type, out_region_start, out_region_end);

    // TODO: Stub until I implement proper direct memory mapping (soon)
    *out_type = 0;
    *out_region_start = start;
    *out_region_end   = start;

    if (dmem_size_map.contains((u64)start))
        *out_region_end = (void*)((u64)start + dmem_size_map[(u64)start]);

    log("out_region_start: %p, out_region_end: %p\n", *out_region_start, *out_region_end);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelAvailableDirectMemorySize(u64 search_start, u64 search_end, size_t alignment, u64* phys_addr_out, size_t* size_out) {
    log("sceKernelAvailableDirectMemorySize(search_start=0x%llx, search_end=0x%llx, alignment=0x%llx, phys_addr_out=*%p, size_out=*%p)\n", search_start, search_end, alignment, phys_addr_out, size_out);

    // TODO
    *phys_addr_out = search_start;
    *size_out = search_end - search_start;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelAvailableFlexibleMemorySize(size_t* size_out) {
    log("sceKernelAvailableFlexibleMemorySize(size_out=*%p)\n", size_out);

    // TODO
    *size_out = 1_GB;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelVirtualQuery(const void* addr, s32 flags, SceKernelVirtualQueryInfo* info, size_t info_size) {
    log("sceKernelVirtualQuery(addr=%p, flags=0x%x, info=*%p, info_size=%d)\n", addr, flags, info, info_size);

    // TODO: This is a very bad stub. I need to properly implement the memory map.
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(addr, &mbi, sizeof(mbi));

    if (mbi.State == MEM_RESERVE) {
        if (flags & 1) {    // SCE_KERNEL_VQ_FIND_NEXT
            void* next = findNextMapped((uptr)addr, 0x8000'0000 + 2000_GB, 4_KB, 1);
            if (!next) return SCE_KERNEL_ERROR_EACCES;

            VirtualQuery(next, &mbi, sizeof(mbi));
        } else return SCE_KERNEL_ERROR_EACCES;
    }

    info->start             = mbi.BaseAddress;
    info->end               = (void*)((uptr)mbi.BaseAddress + mbi.RegionSize);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    bool is_dmem = false;
    if (virt_dmem_map.contains(info->start)) {
        is_dmem = true;
    }

    info->protection = 0;
    info->is_flexible_mem = !is_dmem;
    info->is_direct_mem = is_dmem;
    info->is_stack = 0;
    info->is_pooled_mem = 0;
    info->is_committed = 1;
    if (is_dmem) {
        info->offset = virt_dmem_map[info->start];
    }
    else {
        info->offset = 0;
    }
    info->name[0] = '\0';

    log("info->start                 :    %p\n", info->start);
    log("info->end                   :    %p\n", info->end);
    log("info->protection            :    %d\n", info->protection);
    log("info->is_flexible_mem       :    %d\n", (u8)info->is_flexible_mem);
    log("info->is_direct_mem         :    %d\n", (u8)info->is_direct_mem);
    log("info->is_stack              :    %d\n", (u8)info->is_stack);
    log("info->is_pooled_mem         :    %d\n", (u8)info->is_pooled_mem);
    log("info->is_committed          :    %d\n", (u8)info->is_committed);
    log("info->offset                :    %p\n", info->offset);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelDirectMemoryQuery(const u64 addr, s32 flags, SceKernelDirectMemoryQueryInfo* info, size_t info_size) {
    printf("sceKernelDirectMemoryQuery(addr=%p, flags=%d, info=*%p, info_size=%lld)\n", addr, flags, info, info_size);

    const bool find_next = flags;
    auto map_addr = addr;

    if (!dmem_size_map.contains(addr)) {
        if (!find_next) {
            Helpers::panic("sceKernelDirectMemoryQuery: addr %p is unmapped and SCE_KERNEL_DMQ_FIND_NEXT was not specified (TODO)\n", addr);
        }
        else {
            auto it = dmem_size_map.upper_bound(addr);
            if (it == dmem_size_map.end()) {
                printf("sceKernelDirectMemoryQuery: no next\n");
                return SCE_KERNEL_ERROR_EACCES;
            }

            map_addr = it->first;
        }
    }

    const auto size = dmem_size_map[map_addr];
    info->start = map_addr;
    info->end = map_addr + size;
    info->memory_type = 0 | 3 | 10;

    return SCE_OK;
}

s32 PS4_FUNC sceKernelQueryMemoryProtection(void* addr, void** start, void** end, s32* prot) {
    log("sceKernelQueryMemoryProtection(addr=%p, start=*%p, end=*%p, prot=*%p)\n", addr, start, end, prot);
    
    if (start || end) {
        Helpers::panic("TODO: sceKernelQueryMemoryProtection with start/end");
    }

    // TODO: Stubbed
    *prot = 0x2 | 0x4 | 0x30;   // CPU RWX + GPU RW
    return SCE_OK;
}

void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs) {
    log("mmap(addr=%p, len=0x%llx, prot=0x%x, flags=0x%x, fd=%d, offs=0x%llx)\n", addr, len, prot, flags, fd, offs);

    void* out_addr = nullptr;

    // stdout / stdin / stderr / ?
    if (fd < 0x100)
        fd = -1;

    if (fd == -1) {
#ifdef _WIN32
        out_addr = allocate(SYSTEM_MAPPING_AREA, SYSTEM_MAPPING_AREA + 2000_GB, Helpers::alignUp<size_t>(len, 16_KB), 16_KB);
#else
        Helpers::panic("Unsupported platform\n");
#endif

        // Clear allocated memory
        std::memset(out_addr, 0, len);
    }
    else {
#ifdef _WIN32
        auto& file = FS::getFileFromID(fd);
        if (len == 0) len = FS::getFileSize(fd);
        
        HANDLE file_handle = (HANDLE)_get_osfhandle(_fileno(file.file));
        if (file_handle == INVALID_HANDLE_VALUE) Helpers::panic("kernel_mmap: could not retrieve native file handle for fd %d\n", fd);

        HANDLE mapping = CreateFileMappingW(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!mapping) Helpers::panic("kernel_mmap: CreateFileMappingW failed for fd %d\n", fd);

        out_addr = MapViewOfFile(mapping, FILE_MAP_READ, offs >> 32, offs & 0xffffffff, len);
        if (!out_addr) Helpers::panic("kernel_mmap: MapViewOfFile failed for fd %d\n", fd);
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }

    log("Allocated at %p\n", out_addr);
    return out_addr;
}

s32 PS4_FUNC sceKernelMmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs, void** res) {
    log("sceKernelMmap(addr=%p, len=0x%llx, prot=%d, flags=%d, fd=%d, physical_addr=%p, res=*%p)\n", addr, len, prot, flags, fd, offs, res);

    *res = kernel_mmap(addr, len, prot, flags, fd, offs);   // TODO: Check errors
    return SCE_OK;
}

s32 PS4_FUNC sceKernelBatchMap(SceKernelBatchMapEntry* entries, s32 n_entries, s32* n_processed) {
    log("sceKernelBatchMap(entries=*%p, n_entries=%d, n_processed=*%p) [forwarding to sceKernelBatchMap2]\n", entries, n_entries, n_processed);
    return sceKernelBatchMap2(entries, n_entries, n_processed, SCE_KERNEL_MAP_FIXED);
}

s32 PS4_FUNC sceKernelBatchMap2(SceKernelBatchMapEntry* entries, s32 n_entries, s32* n_processed, s32 flags) {
    log("sceKernelBatchMap2(entries=*%p, n_entries=%d, n_processed=*%p, flags=%d)\n", entries, n_entries, n_processed, flags);

    s32 processed;
    for (processed = 0; processed < n_entries; processed++) {
        const auto& i = processed;
        s32 ret = SCE_OK;

        switch (entries[processed].operation) {
        case SCE_KERNEL_MAP_OP_MAP_DIRECT:      ret = sceKernelMapDirectMemory(&entries[i].start, entries[i].length, entries[i].prot, flags, (void*)entries[i].offset, 0);  break;
        case SCE_KERNEL_MAP_OP_UNMAP:           ret = sceKernelMunmap(entries[i].start, entries[i].length);                                                                 break;
        case SCE_KERNEL_MAP_OP_PROTECT:         Helpers::panic("sceKernelBatchMap2: SCE_KERNEL_MAP_OP_PROTECT (TODO)\n");                                                   break;
        case SCE_KERNEL_MAP_OP_MAP_FLEXIBLE:    ret = sceKernelMapFlexibleMemory(&entries[i].start, entries[i].length, entries[i].prot, flags);                             break;
        case SCE_KERNEL_MAP_OP_TYPE_PROTECT:    printf("sceKernelBatchMap2: SCE_KERNEL_MAP_OP_TYPE_PROTECT (TODO)\n");                                                      break;
        }

        if (ret != SCE_OK) {
            log("sceKernelBatchMap2: interrupted because of error\n");
            break;
        }
    }

    if (n_processed) *n_processed = processed;
    return SCE_OK;
}

SceKernelModule PS4_FUNC sceKernelLoadStartModule(const char* module_path, size_t args, const void* argp, u32 flags, const SceKernelLoadModuleOpt* opt, s32* res) {
    log("sceKernelLoadStartModule(module_path=\"%s\", args=%d, argp=%p, flags=0x%x, opt=*%p, res=*%p)\n", module_path, args, argp, flags, opt, res);

    const fs::path host_module_path = FS::guestPathToHost(module_path);
    if (!fs::exists(host_module_path)) {
        printf("sceKernelLoadStartModule: module %s does not exist\n", module_path);
        return SCE_KERNEL_ERROR_ENOENT;
    }

    // Don't load the module if it was already loaded (not sure if this should return an error?)
    const auto filename = host_module_path.filename().generic_string();
    bool already_loaded = false;
    u32 modid = 0;
    g_app.forEachModule([&](auto mod) -> bool {
        if (mod->filename == filename) {
            already_loaded = true;
            modid = mod->modid;
            return true;
        }
        return false;
    });

    if (already_loaded) {
        log("Module was already loaded\n");
        return modid;
    }

    auto module = PS4::Loader::Linker::loadAndLinkLib(g_app, host_module_path);
    log("Loaded module \"%s\" at %p\n", module->filename.c_str(), module->base_address);
    
    s32 ret = module->init_func(args, argp, (void*)module->proc_param_ptr);   // TODO: libkernel.sprx seems to pass the address of the module_start (nid=0xBaOKcng8g88) symbol as parameter

    std::ofstream log_out;
    log_out.open("log_baseaddress.txt", std::ios::app);
    const auto base_address_str = std::format("{:016p}:\t{}\n", module->base_address, module->filename);
    log_out.write(base_address_str.c_str(), base_address_str.length());
    log_out.flush();
    log_out.close();

    if (res) *res = ret;
    return module->modid;
}

// The signature is probably not correct, but the path is and that's what we care about.
SceKernelModule PS4_FUNC sceKernelLoadStartModuleInternalForMono(const char* module_path, size_t args, const void* argp, u32 flags, const SceKernelLoadModuleOpt* opt, s32* res) {
    return sceKernelLoadStartModule(module_path, args, argp, flags, opt, res);
}

s32 PS4_FUNC sceKernelDlsym(SceKernelModule handle, const char* symbol, void** addr_ptr) {
    log("sceKernelDlsym(handle=%d, symbol=\"%s\", addr_ptr=%p)\n", handle, symbol, addr_ptr);

    // Convert symbol to NID
    const auto nid = Helpers::nameToNid(symbol);

    // Interpret handle 0 as "search all modules".
    // TODO: fix this (use proper sceSysmodule handles)
    if (handle == 0 || handle == 0x10000) {
        Symbol* sym = nullptr;
        g_app.forEachModule([&](auto mod) -> bool {
            sym = mod->findSymbolExport(nid);
            
            // Stop searching if found
            if (sym)
                return true;

            return false;   // Continue searching
        });

        if (!sym) {
            log("sceKernelDlsym: could not find symbol\n");
            return SCE_KERNEL_ERROR_EFAULT;
        }

        *addr_ptr = sym->ptr;
    }
    else {

        auto module = g_app.findModule(handle);
        if (!module) {
            Helpers::panic("sceKernelDlsym: could not find module %d\n", handle);
        }

        auto* sym = module->findSymbolExport(nid);
        if (!sym) {
            log("sceKernelDlsym: could not find symbol\n");
            return SCE_KERNEL_ERROR_EFAULT;
        }

        *addr_ptr = sym->ptr;
    }

    return SCE_OK;
}

// libc.prx HLE

void* PS4_FUNC malloc(size_t size) {
    printf("malloc\n");
    return std::malloc(size);
}

void PS4_FUNC free(void* ptr) {
    printf("free\n");
    std::free(ptr);
}

}