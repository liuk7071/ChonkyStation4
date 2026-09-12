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

static constexpr s32 SCE_DBG_MAX_NAME_LENGTH = 256;
static constexpr s32 SCE_DBG_MAX_SEGMENTS    = 4;
static constexpr s32 SCE_DBG_NUM_FINGERPRINT = 20;

static constexpr s32 KERNEL_RLIMIT_CPU      = 0;       /* maximum cpu time in seconds */
static constexpr s32 KERNEL_RLIMIT_FSIZE    = 1;       /* maximum file size */
static constexpr s32 KERNEL_RLIMIT_DATA     = 2;       /* data size */
static constexpr s32 KERNEL_RLIMIT_STACK    = 3;       /* stack size */
static constexpr s32 KERNEL_RLIMIT_CORE     = 4;       /* core file size */
static constexpr s32 KERNEL_RLIMIT_RSS      = 5;       /* resident set size */
static constexpr s32 KERNEL_RLIMIT_MEMLOCK  = 6;       /* locked-in-memory address space */
static constexpr s32 KERNEL_RLIMIT_NPROC    = 7;       /* number of processes */
static constexpr s32 KERNEL_RLIMIT_NOFILE   = 8;       /* number of open files */
static constexpr s32 KERNEL_RLIMIT_SBSIZE   = 9;       /* maximum size of all socket buffers */
static constexpr s32 KERNEL_RLIMIT_VMEM     = 10;      /* virtual process size (incl. mmap) */
static constexpr s32 KERNEL_RLIMIT_NPTS     = 11;      /* pseudo-terminals */
static constexpr s32 KERNEL_RLIMIT_SWAP     = 12;      /* swap used */
static constexpr s32 KERNEL_RLIMIT_KQUEUES  = 13;      /* kqueues allocated */
static constexpr s32 KERNEL_RLIMIT_UMTXP    = 14;      /* process-shared umtx */
static constexpr s32 KERNEL_RLIMIT_PIPEBUF  = 15;      /* pipes/fifos buffers */
static constexpr s32 KERNEL_RLIMIT_VMM      = 16;      /* virtual machines */

static constexpr s32 SCE_KERNEL_MAP_OP_MAP_DIRECT    = 0;
static constexpr s32 SCE_KERNEL_MAP_OP_UNMAP         = 1;
static constexpr s32 SCE_KERNEL_MAP_OP_PROTECT       = 2;
static constexpr s32 SCE_KERNEL_MAP_OP_MAP_FLEXIBLE  = 3;
static constexpr s32 SCE_KERNEL_MAP_OP_TYPE_PROTECT  = 4;



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

struct SceKernelDirectMemoryQueryInfo {
    u64 start;
    u64 end;
    s32 memory_type;
};

struct SceKernelBatchMapEntry {
    void* start;
    size_t offset;  // off_t
    size_t length;
    u8 prot;
    u8 type;
    u16 reserved;
    s32 operation;
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

struct SceKernelModuleSegmentInfo {
    void* addr;
    u32 size;
    s32 prot;
};

struct SceKernelModuleInfo {
    u64 st_size = sizeof(SceKernelModuleInfo);
    char name[SCE_DBG_MAX_NAME_LENGTH];
    SceKernelModuleSegmentInfo segments[SCE_DBG_MAX_SEGMENTS];
    u32 segment_count;
    u8 fingerprint[SCE_DBG_NUM_FINGERPRINT];
};

struct SceKernelModuleInfoEx {
    u64 st_size;
    char name[SCE_DBG_MAX_NAME_LENGTH];
    s32 id;
    u32 tls_index;
    void* tls_init_addr;
    u32 tls_init_size;
    u32 tls_size;
    u32 tls_offset;
    u32 tls_align;
    void* init_proc_addr;
    void* fini_proc_addr;
    u64 reserved1;
    u64 reserved2;
    void* eh_frame_hdr_addr;
    void* eh_frame_addr;
    u32 eh_frame_hdr_size;
    u32 eh_frame_size;
    SceKernelModuleSegmentInfo segments[SCE_DBG_MAX_SEGMENTS];
    u32 segment_count;
};

struct SceKernelModuleInfoForUnwind {
    u64 st_size;
    char name[256];
    void* eh_frame_hdr_addr;
    void* eh_frame_addr;
    u64 eh_frame_size;
    void* seg0_addr;
    u64 seg0_size;
};

struct SceKernelLoadModuleOpt;
using SceKernelModule = s32;

struct kernel_rlimit {
    u64 rlim_cur;
    u64 rlim_max;
};

s32* PS4_FUNC kernel_error();
s32 PS4_FUNC kernel_getpagesize();
void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx);
s32 PS4_FUNC kernel_nanosleep(SceKernelTimespec* rqtp, SceKernelTimespec* rmtp);
s32 PS4_FUNC sceKernelNanosleep(const SceKernelTimespec* rqtp, SceKernelTimespec* rmtp);
s32 PS4_FUNC sceKernelUsleep(u32 us);
s32 PS4_FUNC sceKernelSleep(u32 s);
s32 PS4_FUNC kernel_clock_gettime(u32 clock_id, SceKernelTimespec* ts);
s32 PS4_FUNC sceKernelClockGettime(u32 clock_id, SceKernelTimespec* ts);
s32 PS4_FUNC sceKernelConvertUtcToLocaltime(time_t time, time_t* local_time, SceKernelTimesec* st, u64* dst_sec);
s32 PS4_FUNC sceKernelConvertLocaltimeToUtc(time_t local_time, s64 unk1, time_t* time, SceKernelTimezone* timezone, u64* dst_sec);
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
s32 PS4_FUNC sceKernelGetModuleInfo2(s32 handle, SceKernelModuleInfo* info);
s32 PS4_FUNC sceKernelGetModuleList2(s32* handles, u64 n_handles, u64* out_n_handles);
s32 PS4_FUNC sceKernelGetModuleInfoFromAddr(void* addr, s32 flags, SceKernelModuleInfoEx* info);
s32 PS4_FUNC sceKernelGetModuleInfoForUnwind(void* addr, s32 flags, SceKernelModuleInfoForUnwind* info);
s32 PS4_FUNC sceKernelDebugRaiseExceptionOnReleaseMode(u32 error);
s32 PS4_FUNC kernel_getrlimit(s32 resource, kernel_rlimit* rlim);
s64 PS4_FUNC sceLibcMspaceCreateForMonoMutex(s64 unk1, s32 unk2, s32 unk3, s64 unk4);
s32 PS4_FUNC sceKernelGetCompiledSdkVersion(s32* ver);
s32 PS4_FUNC sceKernelGetProcessName(s64 pid, char* name);
s32 PS4_FUNC sceKernelGetDataTransferMode(s32* mode);
s32 PS4_FUNC sceKernelGetBackupRestoreMode(s32* mode);
s32 PS4_FUNC kernel_getargc();
const char** PS4_FUNC kernel_getargv();

s32 PS4_FUNC kernel_getpid();
s32 PS4_FUNC kernel_sched_get_priority_max();
s32 PS4_FUNC kernel_sched_get_priority_min();
s32 PS4_FUNC sigfillset();
void PS4_FUNC sceKernelDebugOutText(s64 unknown, char* text);

s32 PS4_FUNC __sys_regmgr_call();

// Signal
struct Siginfo;
struct Sigset {
    u32 bits[4];
};
struct Sigaction {
    union {
        void PS4_FUNC (*sa_handler)(int);
        void PS4_FUNC (*sa_sigaction)(int, Siginfo*, void*);
    } __sigaction_handler;
    s32 sa_flags;
    Sigset sa_mask;
};

s32 PS4_FUNC kernel_sigaction(s32 sig, Sigaction* act, Sigaction* oact);
s32 PS4_FUNC kernel_sigprocmask();

// Shared memory
s32 PS4_FUNC kernel_shm_open(const char* path, s32 flags, s32 /* mode_t */ mode);

// ipmi
s32 PS4_FUNC ipmimgr_call(s64 cmd, s64 unk2, u32* res, u8* args, size_t arg_size_bytes, u64 unk3);

// Memory
s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align);
s32 PS4_FUNC sceKernelMapNamedDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align, const char* name);
s32 PS4_FUNC sceKernelMapFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags);
s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name);
s32 PS4_FUNC sceKernelMapNamedSystemFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name);
s32 PS4_FUNC sceKernelReserveVirtualRange(void** addr, size_t len, s32 flags, size_t align);
s32 PS4_FUNC sceKernelReleaseDirectMemory(void* addr, size_t len);
s32 PS4_FUNC sceKernelCheckedReleaseDirectMemory(void* addr, size_t len);
s32 PS4_FUNC sceKernelMunmap(void* addr, size_t len);
s32 PS4_FUNC kernel_munmap(void* addr, size_t len);
size_t PS4_FUNC sceKernelGetDirectMemorySize();
s32 PS4_FUNC sceKernelConfiguredFlexibleMemorySize(size_t* out_size);
s32 PS4_FUNC sceKernelGetDirectMemoryType(void* start, s32* out_type, void** out_region_start, void** out_region_end);
s32 PS4_FUNC sceKernelAvailableDirectMemorySize(u64 search_start, u64 search_end, size_t alignment, u64* phys_addr_out, size_t* size_out);
s32 PS4_FUNC sceKernelAvailableFlexibleMemorySize(size_t* size_out);
s32 PS4_FUNC sceKernelVirtualQuery(const void* addr, s32 flags, SceKernelVirtualQueryInfo* info, size_t info_size);
s32 PS4_FUNC sceKernelDirectMemoryQuery(const u64 addr, s32 flags, SceKernelDirectMemoryQueryInfo* info, size_t info_size);
s32 PS4_FUNC sceKernelQueryMemoryProtection(void* addr, void** start, void** end, s32* prot);
void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs);
s32 PS4_FUNC sceKernelMmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs, void** res);
s32 PS4_FUNC sceKernelBatchMap(SceKernelBatchMapEntry* entries, s32 n_entries, s32* n_processed);
s32 PS4_FUNC sceKernelBatchMap2(SceKernelBatchMapEntry* entries, s32 n_entries, s32* n_processed, s32 flags);

// Module
SceKernelModule PS4_FUNC sceKernelLoadStartModule(const char* module_path, size_t args, const void* argp, u32 flags, const SceKernelLoadModuleOpt* opt, s32* res);
SceKernelModule PS4_FUNC sceKernelLoadStartModuleInternalForMono(const char* module_path, size_t args, const void* argp, u32 flags, const SceKernelLoadModuleOpt* opt, s32* res);
s32 PS4_FUNC sceKernelDlsym(SceKernelModule handle, const char* symbol, void** addr_ptr);

// libc.prx HLE
void* PS4_FUNC malloc(size_t size);
void PS4_FUNC free(void* ptr);

}   // End namespace PS4::OS::Libs::Kernel