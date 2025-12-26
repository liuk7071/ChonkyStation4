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
    const auto ret = FS::open(path, mode);
    if (!ret) {
        *Kernel::kernel_error() = POSIX_ENOENT;
        return -1;
    }
    return ret;
}

s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode) {
    const auto res = kernel_open(path, flags, mode);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s64 PS4_FUNC kernel_lseek(s32 fd, s64 offset, s32 whence) {
    log("lseek(fd=%d, offset=%lld, whence=%d)\n", fd, offset, whence);
    auto lock = FS::getFileLock(fd);
    return FS::seek(fd, offset, whence);
}

s64 PS4_FUNC sceKernelLseek(s32 fd, s64 offset, s32 whence) {
    const auto res = kernel_lseek(fd, offset, whence);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s64 PS4_FUNC kernel_read(s32 fd, u8* buf, u64 size) {
    log("read(fd=%d, buf=%p, size=%lld)\n", fd, buf, size);
    auto lock = FS::getFileLock(fd);
    return FS::read(fd, buf, size);
}

s64 PS4_FUNC sceKernelRead(s32 fd, u8* buf, u64 size) {
    const auto res = kernel_read(fd, buf, size);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s64 PS4_FUNC kernel_pread(s32 fd, u8* buf, u64 size, s64 offset) {
    log("pread(fd=%d, buf=%p, size=%lld, offset=%lld)\n", fd, buf, size, offset);
    auto lock = FS::getFileLock(fd);

    // Save old seek position
    const auto old_pos = FS::tell(fd);
    // Seek to offset and read
    FS::seek(fd, offset, SEEK_SET);
    const auto ret = FS::read(fd, buf, size);
    // Restore old seek position
    FS::seek(fd, old_pos, SEEK_SET);
    return ret;
}

s64 PS4_FUNC sceKernelPread(s32 fd, u8* buf, u64 size, s64 offset) {
    const auto res = kernel_pread(fd, buf, size, offset);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s64 PS4_FUNC kernel_write(s32 fd, u8* buf, u64 size) {
    //log("_write(fd=%d, buf=%p, size=%d)\n", fd, buf, size);

    if (fd != 0 && fd != 1 && fd != 2) {
        Helpers::panic("_write: fd is not stdin or stdout or stderr (TODO)\n");
    }

    for (char* ptr = (char*)buf; ptr < (char*)buf + size; ptr++)
        std::putc(*ptr, stdout);
    return size;
}

s64 PS4_FUNC sceKernelWrite(s32 fd, u8* buf, u64 size) {
    const auto res = kernel_write(fd, buf, size);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s64 PS4_FUNC kernel_writev(s32 fd, SceKernelIovec* iov, int iovcnt) {
    //log("_writev(fd=%d, iov=%p, iovcnt=%d)\n", fd, iov, iovcnt);

    if (fd != 0 && fd != 1) {
        Helpers::panic("_writev: fd is not stdin or stdout (TODO)\n");
    }

    size_t written = 0;
    for (int i = 0; i < iovcnt; i++) {
        char* ptr = nullptr;
        for (ptr = (char*)iov[i].iov_base; ptr < (char*)iov[i].iov_base + iov[i].iov_len; ptr++)
            std::putc(*ptr, stdout);

        written += iov[i].iov_len;
    }

    return written;
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
    stat->st_blksize = 512;    // TODO: ?
    stat->st_blocks = (stat->st_size + stat->st_blksize - 1) / stat->st_blksize;    // TODO: ?
    return 0;
}

s32 PS4_FUNC sceKernelStat(const char* path, SceKernelStat* stat) {
    const auto res = kernel_stat(path, stat);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC kernel_fstat(s32 fd, SceKernelStat* stat) {
    log("fstat(fd=%d, stat=*%p)\n", fd, stat);
    auto lock = FS::getFileLock(fd);

    // TODO: Avoid duplicated code from stat
    std::memset(stat, 0, sizeof(SceKernelStat));
    const bool is_dir = FS::isDirectory(fd);
    stat->st_mode = FS::SCE_KERNEL_S_IRWU;  // read-write
    stat->st_mode |= !is_dir ? FS::SCE_KERNEL_S_IFREG : FS::SCE_KERNEL_S_IFDIR;
    stat->st_uid = 0;
    stat->st_gid = 0;
    // TODO: time
    stat->st_size = FS::getFileSize(fd);
    stat->st_blksize = 512;    // TODO: ?
    stat->st_blocks = (stat->st_size + stat->st_blksize - 1) / stat->st_blksize;
    return 0;
}

s32 PS4_FUNC sceKernelFstat(s32 fd, SceKernelStat* stat) {
    const auto res = kernel_fstat(fd, stat);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC kernel_close(s32 fd) {
    log("close(fd=%d)\n", fd);
    
    if (!FS::exists(fd)) {
        *Kernel::kernel_error() = POSIX_ENOENT;
        return -1;
    }

    // The game wouldn't close the file while other threads are accessing it... but just to be sure
    // Notice the scope because after FS::close the file mutex will be invalid
    {
        auto lock = FS::getFileLock(fd);
    }

    FS::close(fd);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelClose(s32 fd) {
    const auto res = kernel_close(fd);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

}   // End namespace PS4::OS::Libs::Kernel