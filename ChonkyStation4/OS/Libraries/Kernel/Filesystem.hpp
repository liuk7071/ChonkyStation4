#pragma once

#include <Common.hpp>
#include <deque>


namespace PS4::OS::Libs::Kernel {

struct SceKernelTimespec {
    s64 tv_sec;
    s64 tv_nsec;
};

struct SceKernelStat {
    u32 st_dev;
    u32 st_ino;
    u16 st_mode;
    u16 st_nlink;
    u32 st_uid;
    u32 st_gid;
    u32 st_rdev;
    SceKernelTimespec st_atim;
    SceKernelTimespec st_mtim;
    SceKernelTimespec st_ctim;
    s64 st_size;
    s64 st_blocks;
    u32 st_blksize;
    u32 st_flags;
    u32 st_gen;
    s32 st_lspare;
    SceKernelTimespec st_birthtim;
    u32 : (8 / 2) * (16 - static_cast<int>(sizeof(SceKernelTimespec)));
    u32 : (8 / 2) * (16 - static_cast<int>(sizeof(SceKernelTimespec)));
};

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode);
s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode);
s64 PS4_FUNC kernel_lseek(s32 fd, s64 offset, s32 whence);
s64 PS4_FUNC sceKernelLseek(s32 fd, s64 offset, s32 whence);
s64 PS4_FUNC kernel_read(s32 fd, u8* buf, u64 size);
s64 PS4_FUNC sceKernelRead(s32 fd, u8* buf, u64 size);
s32 PS4_FUNC kernel_stat(const char* path, SceKernelStat* stat);
s32 PS4_FUNC sceKernelStat(const char* path, SceKernelStat* stat);
s32 PS4_FUNC kernel_close(s32 fd);
s32 PS4_FUNC sceKernelClose(s32 fd);

};  // End namespace PS4::OS::Libs::Kernel