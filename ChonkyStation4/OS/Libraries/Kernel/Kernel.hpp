#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::Kernel {

void init(Module& module);

static constexpr s32 SCE_KERNEL_CLOCK_REALTIME = 0;
static constexpr s32 SCE_KERNEL_CLOCK_VIRTUAL = 1;
static constexpr s32 SCE_KERNEL_CLOCK_PROF = 2;
static constexpr s32 SCE_KERNEL_CLOCK_MONOTONIC = 4;
static constexpr s32 SCE_KERNEL_CLOCK_UPTIME = 5;
static constexpr s32 SCE_KERNEL_CLOCK_UPTIME_PRECISE = 7;
static constexpr s32 SCE_KERNEL_CLOCK_UPTIME_FAST = 8;
static constexpr s32 SCE_KERNEL_CLOCK_REALTIME_PRECISE = 9;
static constexpr s32 SCE_KERNEL_CLOCK_REALTIME_FAST = 10;
static constexpr s32 SCE_KERNEL_CLOCK_MONOTONIC_PRECISE = 11;
static constexpr s32 SCE_KERNEL_CLOCK_MONOTONIC_FAST = 12;
static constexpr s32 SCE_KERNEL_CLOCK_SECOND = 13;
static constexpr s32 SCE_KERNEL_CLOCK_THREAD_CPUTIME_ID = 14;
static constexpr s32 SCE_KERNEL_CLOCK_PROCTIME = 15;
static constexpr s32 SCE_KERNEL_CLOCK_EXT_NETWORK = 16;
static constexpr s32 SCE_KERNEL_CLOCK_EXT_DEBUG_NETWORK = 17;
static constexpr s32 SCE_KERNEL_CLOCK_EXT_AD_NETWORK = 18;
static constexpr s32 SCE_KERNEL_CLOCK_EXT_RAW_NETWORK = 19;

static constexpr s32 SCE_KERNEL_MAP_FIXED = 0x10;

struct TLSIndex {
    u64 modid;
    u64 offset;
};

s32* PS4_FUNC kernel_error();
void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx);
void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs);
s32 PS4_FUNC kernel_nanosleep(SceKernelTimespec* rqtp, SceKernelTimespec* rmtp);
s32 PS4_FUNC sceKernelUsleep(u32 us);
s32 PS4_FUNC sceKernelSleep(u32 s);
s32 PS4_FUNC kernel_clock_gettime(u32 clock_id, SceKernelTimespec* ts);
s32 PS4_FUNC sceKernelClockGettime(u32 clock_id, SceKernelTimespec* ts);
s32 PS4_FUNC kernel_gettimeofday(SceKernelTimeval* tv, SceKernelTimezone* tz);
s32 PS4_FUNC sceKernelGettimezone(SceKernelTimezone* tz);
u64 PS4_FUNC sceKernelGetProcessTime();
u64 PS4_FUNC sceKernelReadTsc();
s32 PS4_FUNC sceKernelIsNeoMode();
void* PS4_FUNC sceKernelGetProcParam();
s32 PS4_FUNC sceKernelGetProcessType();
void PS4_FUNC _sceKernelRtldSetApplicationHeapAPI(void* api[]);
u64 PS4_FUNC sceKernelGetTscFrequency();

s32 PS4_FUNC kernel_getpid();
s32 PS4_FUNC sigfillset();

s32 PS4_FUNC __sys_regmgr_call();

// Memory
s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align);
s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name);
s32 PS4_FUNC sceKernelMunmap(void* addr, size_t len);
s32 PS4_FUNC sceKernelGetDirectMemorySize();

}   // End namespace PS4::OS::Libs::Kernel