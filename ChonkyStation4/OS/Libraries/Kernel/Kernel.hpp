#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::Kernel {

void init(Module& module);

struct KernelIovec {
    void* iov_base;
    size_t iov_len;
};

struct TLSIndex {
    u64 modid;
    u64 offset;
};

s32* PS4_FUNC ___error();
void* PS4_FUNC __tls_get_addr(TLSIndex* tls_idx);
void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs);
size_t PS4_FUNC kernel_writev(s32 fd, KernelIovec* iov, int iovcnt);
size_t PS4_FUNC kernel_write(s32 fd, const void* buf, size_t size);
s32 PS4_FUNC sceKernelUsleep(u32 us);
s32 PS4_FUNC sceKernelIsNeoMode();
void* PS4_FUNC sceKernelGetProcParam();
s32 PS4_FUNC sceKernelGetProcessType();
void PS4_FUNC _sceKernelRtldSetApplicationHeapAPI(void* api[]);

s32 PS4_FUNC kernel_getpid();
s32 PS4_FUNC sigfillset();

s32 PS4_FUNC __sys_regmgr_call();

// Memory
s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelAllocateDirectMemory(void* search_start, void* search_end, size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align);
s32 PS4_FUNC sceKernelMapNamedFlexibleMemory(void** addr, size_t len, s32 prot, s32 flags, const char* name);
s32 PS4_FUNC sceKernelGetDirectMemorySize();

}   // End namespace PS4::OS::Libs::Kernel