// Adapted from ChonkyStation3

#pragma once

#include <Common.hpp>
#include <unordered_map>

namespace PS4::FS {

static constexpr s32 SCE_KERNEL_O_RDONLY = 0;
static constexpr s32 SCE_KERNEL_O_WRONLY = 1;
static constexpr s32 SCE_KERNEL_O_RDWR   = 2;

static constexpr s32 SCE_KERNEL_O_NONBLOCK  = 0x4;
static constexpr s32 SCE_KERNEL_O_APPEND    = 0x8;
static constexpr s32 SCE_KERNEL_O_SYNC      = 0x80;
static constexpr s32 SCE_KERNEL_O_CREAT     = 0x200;
static constexpr s32 SCE_KERNEL_O_TRUNC     = 0x400;
static constexpr s32 SCE_KERNEL_O_EXCL      = 0x800;
static constexpr s32 SCE_KERNEL_O_DSYNC     = 0x1000;
static constexpr s32 SCE_KERNEL_O_DIRECT    = 0x10000;
static constexpr s32 SCE_KERNEL_O_DIRECTORY = 0x20000;

enum class Device {
    APP0,
    DEV,
    INVALID
};

struct File {
    FILE* file;
    fs::path path;
    fs::path guest_path;
    u32 flags = 0;
};

struct Directory {
    fs::path path;
    int cur = 0;
};

void mount(Device device, fs::path path);
void umount(Device device);
void init();
u64 open(fs::path path, u32 flags = 0);
u64 opendir(fs::path path);
void close(u64 file_id);
void closedir(u64 file_id);
u64 read(u64 file_id, u8* buf, u64 size);
u64 write(u64 file_id, u8* buf, u64 size);
u64 seek(u64 file_id, s64 offs, u32 mode);
u64 tell(u64 file_id);
bool mkdir(fs::path path);
u64 getFileSize(u64 file_id);
u64 getFileSize(fs::path path);
bool isDirectory(u64 file_id);
bool isDirectory(fs::path path);
bool exists(fs::path path);
File& getFileFromID(u32 id);
Directory& getDirFromID(u32 id);
bool isDeviceMounted(Device device);
bool isDeviceMounted(fs::path path);
fs::path guestPathToHost(fs::path path);
Device getDeviceFromPath(fs::path path);
bool isValidDevice(fs::path path);
static std::string deviceToString(Device device);
static Device stringToDevice(std::string device);

};  // End namespace PS4::FS
