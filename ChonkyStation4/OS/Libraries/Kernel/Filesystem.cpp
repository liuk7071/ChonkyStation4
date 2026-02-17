#include "Filesystem.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <OS/Filesystem.hpp>
#ifdef _MSC_VER
#include <intrin.h>
#define RETURN_ADDRESS() _ReturnAddress()
#else
#define RETURN_ADDRESS() _builtin_return_address(0)
#endif


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_filesystem);

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode) {
    log("_open(path=\"%s\", flags=%d, mode=%o)\n", path, flags, mode);
    // TODO: mode

    if (!FS::isDeviceMounted(path)) {
        log("WARNING: Device not mounted\n");
        *Kernel::kernel_error() = POSIX_ENOENT;
        return -1;
    }

    u32 err = 0;
    const auto ret = FS::open(path, err, flags);
    if (!ret) {
        *Kernel::kernel_error() = err;
        return -1;
    }
    return ret;
}

s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode) {
    const auto res = kernel_open(path, flags, mode);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC sceKernelCheckReachability(const char* path) {
    return FS::exists(path) ? SCE_OK : SCE_KERNEL_ERROR_ENOENT;
}

s32 PS4_FUNC kernel_mkdir(const char* path, u16 mode) {
    log("mkdir(path=\"%s\", mode=%o)\n", path, mode);

    // Parent directory must exist
    if (!fs::exists(fs::path(path).parent_path())) {
        *Kernel::kernel_error() = POSIX_ENOENT;
        return -1;
    }

    // Error if directory already existed
    if (!FS::mkdir(path)) {
        *Kernel::kernel_error() = POSIX_EEXIST;
        return -1;
    }

    return 0;
}

s32 PS4_FUNC sceKernelMkdir(const char* path, u16 mode) {
    const auto res = kernel_mkdir(path, mode);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s64 PS4_FUNC kernel_lseek(s32 fd, s64 offset, s32 whence) {
    log("lseek(fd=%d, offset=%lld, whence=%d)\n", fd, offset, whence);

    if (!FS::exists(fd)) {
        log("File %d does not exist\n", fd);
        *Kernel::kernel_error() = POSIX_EBADF;
        return -1;
    }

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

    if (fd == 0 || fd == 1 || fd == 2) {
        // Redirect stout/stdin/stderr
        for (char* ptr = (char*)buf; ptr < (char*)buf + size; ptr++)
            std::putc(*ptr, stdout);
        return size;
    }

    auto lock = FS::getFileLock(fd);
    return FS::write(fd, buf, size);
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

s32 PS4_FUNC kernel_ftruncate(s32 fd, s64 len) {
    log("ftruncate(fd=%d, len=0x%llx)\n", fd, len);
    
    auto lock = FS::getFileLock(fd);
    auto& file = FS::getFileFromID(fd);
    fs::resize_file(file.path, len);
    return 0;
}

s32 PS4_FUNC sceKernelFtruncate(s32 fd, s64 len) {
    log("sceKernelFtruncate(fd=%d, len=0x%llx)\n", fd, len);
    const auto res = kernel_ftruncate(fd, len);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC kernel_stat(const char* path, SceKernelStat* stat) {
    log("stat(path=\"%s\", stat=*%p)\n", path, stat);

    if (!FS::isDeviceMounted(path)) {
        log("WARNING: Device not mounted\n");
        *Kernel::kernel_error() = POSIX_ENOENT;
        return -1;
    }

    // Check if the path is valid
    // TODO: Is this the right error or should I also return ENOENT in this case?
    if (FS::guestPathToHost(path).empty()) {
        *Kernel::kernel_error() = POSIX_EINVAL;
        return -1;
    }

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
    stat->st_blocks = (stat->st_size + stat->st_blksize - 1) / stat->st_blksize;
    return 0;
}

s32 PS4_FUNC sceKernelStat(const char* path, SceKernelStat* stat) {
    const auto res = kernel_stat(path, stat);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC kernel_fstat(s32 fd, SceKernelStat* stat) {
    log("fstat(fd=%d, stat=*%p)\n", fd, stat);
    
    if (!FS::exists(fd)) {
        log("File ID does not exist\n");
        *Kernel::kernel_error() = POSIX_EBADF;
        return -1;
    }

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

s32 PS4_FUNC kernel_getdents(s32 fd, char* buf, s32 n_bytes) {
    log("getdents(fd=%d, buf=%p, n_bytes=%d)\n", fd, buf, n_bytes);
    auto lock = FS::getFileLock(fd);
    auto& file = FS::getFileFromID(fd);

    size_t written_size = 0;
    const int n_records = n_bytes / sizeof(FS::SceKernelDirent);
    for (int i = 0; i < n_records; i++) {
        if (file.cur_dirent >= file.dirents.size()) break;

        std::memcpy(buf, &file.dirents[file.cur_dirent], sizeof(FS::SceKernelDirent));
        file.cur_dirent++;
        written_size += sizeof(FS::SceKernelDirent);
    }

    return written_size;
}

s32 PS4_FUNC sceKernelGetdents(s32 fd, char* buf, s32 n_bytes) {
    const auto res = kernel_getdents(fd, buf, n_bytes);
    if (res < 0) return Error::posixToSce(*Kernel::kernel_error());
    return res;
}

s32 PS4_FUNC kernel_getdirentries(s32 fd, char* buf, s32 n_bytes, s64* basep) {
    log("getdirentries(fd=%d, buf=%p, n_bytes=%d, basep=*%p)\n", fd, buf, n_bytes, basep);

    if (basep) {
        Helpers::panic("TODO: getdirentries basep\n");
    }

    kernel_getdents(fd, buf, n_bytes);
    return 0;
}

s32 PS4_FUNC sceKernelGetdirentries(s32 fd, char* buf, s32 n_bytes, s64* basep) {
    const auto res = kernel_getdirentries(fd, buf, n_bytes, basep);
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