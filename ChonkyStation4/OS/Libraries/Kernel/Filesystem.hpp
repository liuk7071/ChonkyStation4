#pragma once

#include <Common.hpp>
#include <deque>


namespace PS4::OS::Libs::Kernel {

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode);
s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode);
s64 PS4_FUNC kernel_lseek(s32 fd, s64 offset, s32 whence);
s64 PS4_FUNC sceKernelLseek(s32 fd, s64 offset, s32 whence);
s64 PS4_FUNC kernel_read(s32 fd, u8* buf, u64 size);
s64 PS4_FUNC sceKernelRead(s32 fd, u8* buf, u64 size);
s64 PS4_FUNC kernel_close(s32 fd);
s64 PS4_FUNC sceKernelClose(s32 fd);

};  // End namespace PS4::OS::Libs::Kernel