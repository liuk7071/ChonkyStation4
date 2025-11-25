#include "Filesystem.hpp"
#include <Logger.hpp>
#include <OS/Filesystem.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_filesystem);

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode) {
    log("_open(path=\"%s\", flags=%d, mode=%o)\n", path, flags, mode);
    // TODO: mode
    return PS4::FS::open(path, mode);
}

s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode) {
    s32 ret = kernel_open(path, flags, mode);
    // TODO: Properly handle return values.
    // POSIX errors are different from the SCE_ ones
    return ret;
}

s64 PS4_FUNC kernel_lseek(s32 fd, s64 offset, s32 whence) {
    log("lseek(fd=%d, offset=%lld, whence=%d)\n", fd, offset, whence);
    return FS::seek(fd, offset, whence);
}

s64 PS4_FUNC sceKernelLseek(s32 fd, s64 offset, s32 whence) {
    s32 ret = kernel_lseek(fd, offset, whence);
    // TODO: Read sceKernelOpen
    return ret;
}

s64 PS4_FUNC kernel_read(s32 fd, u8* buf, u64 size) {
    log("read(fd=%d, buf=%p, size=%lld)\n", fd, buf, size);
    return FS::read(fd, buf, size);
}

s64 PS4_FUNC sceKernelRead(s32 fd, u8* buf, u64 size) {
    s32 ret = kernel_read(fd, buf, size);
    // TODO: Read sceKernelOpen
    return ret;
}

s64 PS4_FUNC kernel_close(s32 fd) {
    log("close(fd=%d)\n", fd);
    
    FS::close(fd);
    return SCE_OK;
}

s64 PS4_FUNC sceKernelClose(s32 fd) {
    s32 ret = kernel_close(fd);
    // TODO: Read sceKernelOpen
    return ret;
}

}   // End namespace PS4::OS::Libs::Kernel