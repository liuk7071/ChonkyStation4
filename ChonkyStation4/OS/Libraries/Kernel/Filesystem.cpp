#include "Filesystem.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <OS/Filesystem.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_filesystem);

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode) {
    log("_open(path=\"%s\", flags=%d, mode=%o)\n", path, flags, mode);
    // TODO: mode
    return FS::open(path, mode);
}

s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode) {
    const s32 ret = kernel_open(path, flags, mode);
    return (ret >= 0) ? ret : Error::posixToSce(ret);
}

s64 PS4_FUNC kernel_lseek(s32 fd, s64 offset, s32 whence) {
    log("lseek(fd=%d, offset=%lld, whence=%d)\n", fd, offset, whence);
    return FS::seek(fd, offset, whence);
}

s64 PS4_FUNC sceKernelLseek(s32 fd, s64 offset, s32 whence) {
    const s32 ret = kernel_lseek(fd, offset, whence);
    return (ret >= 0) ? ret : Error::posixToSce(ret);
}

s64 PS4_FUNC kernel_read(s32 fd, u8* buf, u64 size) {
    log("read(fd=%d, buf=%p, size=%lld)\n", fd, buf, size);
    return FS::read(fd, buf, size);
}

s64 PS4_FUNC sceKernelRead(s32 fd, u8* buf, u64 size) {
    const s32 ret = kernel_read(fd, buf, size);
    return (ret >= 0) ? ret : Error::posixToSce(ret);
}

s32 PS4_FUNC kernel_stat(const char* path, SceKernelStat* stat) {
    log("stat(path=\"%s\", stat=*%p)\n", path, stat);

    if (!FS::exists(path)) {
        *Kernel::kernel_error() = POSIX_ENOENT;
        return -1;
    }

    std::memset(stat, 0, sizeof(SceKernelStat));
    const bool is_dir = FS::isDirectory(path);
    stat->st_mode = FS::SCE_KERNEL_S_IRWU;  // read-write
    stat->st_mode |= !is_dir ? FS::SCE_KERNEL_S_IFREG : FS::SCE_KERNEL_S_IFDIR;
    stat->st_uid = 0;
    stat->st_gid = 0;
    // TODO: time
    stat->st_size = FS::getFileSize(path);
    stat->st_blksize = 4096;    // TODO: ?
    return 0;
}

s32 PS4_FUNC sceKernelStat(const char* path, SceKernelStat* stat) {
    return Error::posixToSce(kernel_stat(path, stat));
}

s32 PS4_FUNC kernel_close(s32 fd) {
    log("close(fd=%d)\n", fd);
    
    FS::close(fd);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelClose(s32 fd) {
    return Error::posixToSce(kernel_close(fd));
}

}   // End namespace PS4::OS::Libs::Kernel