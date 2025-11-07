#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::Kernel {

void init(Module& module);

struct KernelIovec {
    void* iov_base;
    size_t iov_len;
};

s32* PS4_FUNC ___error();
void* PS4_FUNC kernel_mmap(void* addr, size_t len, s32 prot, s32 flags, s32 fd, s64 offs);
size_t PS4_FUNC kernel_writev(s32 fd, KernelIovec* iov, int iovcnt);
size_t PS4_FUNC kernel_write(s32 fd, const void* buf, size_t size);
s32 PS4_FUNC sceKernelUsleep(u32 us);
s32 PS4_FUNC sceKernelIsNeoMode();

// Memory
s32 PS4_FUNC sceKernelAllocateMainDirectMemory(size_t size, size_t align, s32 mem_type, void** out_addr);
s32 PS4_FUNC sceKernelMapDirectMemory(void** addr, size_t len, s32 prot, s32 flags, void* dmem_start, size_t align);

}   // End namespace PS4::OS::Libs::Kernel