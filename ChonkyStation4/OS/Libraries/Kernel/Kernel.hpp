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

struct SceKernelVirtualQueryInfo {
    void* start;
    void* end;
    u64 offset;
    s32 protection;
    s32 memory_type;
    u8 is_flexible_mem : 1;
    u8 is_direct_mem : 1;
    u8 is_stack : 1;
    u8 is_pooled_mem : 1;
    u8 is_committed : 1;
    char name[32];
};

struct SceKernelTitleWorkaround {
    s32 version;
    s32 align;
    u64 ids[2];
};

struct SceKernelAppInfo {
    s32 app_id;
    s32 mmap_flags;
    s32 attribute_exe;
    s32 attribute2;
    char cusa_name[10];
    u8 debug_level;
    u8 slv_flags;
    u8 mini_app_dmem_flags;
    u8 render_mode;
    u8 mdbg_out;
    u8 required_hdcp_type;
    u64 preload_prx_flags;
    s32 attribute1;
    s32 has_param_sfo;
    SceKernelTitleWorkaround title_workaround;
};

struct SceKernelSwVersion {
    u64 struct_size;
    char text[0x1c];
    u32 hex;
};

struct SceKernelLoadModuleOpt;
using SceKernelModule = s32;

s32* PS4_FUNC kernel_error();
s32 PS4_FUNC kernel_getpagesize();
void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx);
s32 PS4_FUNC kernel_nanosleep(SceKernelTimespec* rqtp, SceKernelTimespec* rmtp);
s32 PS4_FUNC sceKernelNanosleep(const SceKernelTimespec* rqtp, SceKernelTimespec* rmtp);
s32 PS4_FUNC sceKernelUsleep(u32 us);
s32 PS4_FUNC sceKernelSleep(u32 s);
s32 PS4_FUNC kernel_clock_gettime(u32 clock_id, SceKernelTimespec* ts);
s32 PS4_FUNC sceKernelClockGettime(u32 clock_id, SceKernelTimespec* ts);
s32 PS4_FUNC kernel_gettimeofday(SceKernelTimeval* tv, SceKernelTimezone* tz);
s32 PS4_FUNC sceKernelGettimeofday(SceKernelTimeval* tv);
s32 PS4_FUNC sceKernelGettimezone(SceKernelTimezone* tz);
u64 PS4_FUNC sceKernelGetProcessTime();
u64 PS4_FUNC sceKernelGetProcessTimeCounter();
u64 PS4_FUNC sceKernelGetProcessTimeCounterFrequency();
void PS4_FUNC kernel_sched_yield();
u64 PS4_FUNC sceKernelReadTsc();
s32 PS4_FUNC sceKernelIsNeoMode();
void* PS4_FUNC sceKernelGetProcParam();
s32 PS4_FUNC sceKernelGetProcessType();
void PS4_FUNC _sceKernelRtldSetApplicationHeapAPI(void* api[]);
u64 PS4_FUNC sceKernelGetTscFrequency();
s32 PS4_FUNC sceKernelGetAppInfo(s32 pid, SceKernelAppInfo* app_info);
s32 PS4_FUNC sceKernelTitleWorkaroundIsEnabled(SceKernelTitleWorkaround* workaround, s32 bit, s32* result);
s32 PS4_FUNC sceKernelGetSystemSwVersion(SceKernelSwVersion* ver);

s32 PS4_FUNC kernel_getpid();
s32 PS4_FUNC kernel_sched_get_priority_max();
s32 PS4_FUNC kernel_sched_get_priority_min();
s32 PS4_FUNC sigfillset();
void PS4_FUNC sceKernelDebugOutText(s64 unknown, char* text);

s32 PS4_FUNC __sys_regmgr_call();

// Memory
s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align);
s32 PS4_FUNC sceKernelMapNamedDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align, const char* name);
s32 PS4_FUNC sceKernelMapFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags);
s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name);
s32 PS4_FUNC sceKernelReserveVirtualRange(void** addr, size_t len, s32 flags, size_t align);
s32 PS4_FUNC sceKernelReleaseDirectMemory(void* addr, size_t len);
s32 PS4_FUNC sceKernelCheckedReleaseDirectMemory(void* addr, size_t len);
s32 PS4_FUNC sceKernelMunmap(void* addr, size_t len);
s32 PS4_FUNC kernel_munmap(void* addr, size_t len);
size_t PS4_FUNC sceKernelGetDirectMemorySize();
s32 PS4_FUNC sceKernelVirtualQuery(const void* addr, s32 flags, SceKernelVirtualQueryInfo* info, size_t info_size);
s32 PS4_FUNC sceKernelQueryMemoryProtection(void* addr, void** start, void** end, s32* prot);
void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs);

// Module
SceKernelModule PS4_FUNC sceKernelLoadStartModule(const char* module_path, size_t args, const void* argp, u32 flags, const SceKernelLoadModuleOpt* opt, s32* res);
s32 PS4_FUNC sceKernelDlsym(SceKernelModule handle, const char* symbol, void** addr_ptr);

// libc.prx HLE
void* PS4_FUNC malloc(size_t size);
void PS4_FUNC free(void* ptr);

}   // End namespace PS4::OS::Libs::Kernel