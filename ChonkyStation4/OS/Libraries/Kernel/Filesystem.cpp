#include "Filesystem.hpp"
#include <Logger.hpp>
#include <OS/SceObj.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_filesystem);

// Bad stub for now until I implement filesystem properly
struct File : SceObj {
    // TODO
};

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode) {
    log("_open(path=\"%s\", flags=%d, mode=%o) TODO\n", path, flags, mode);
    auto file = PS4::OS::make<File>();
    return file->handle;
}

s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode) {
    s32 ret = kernel_open(path, flags, mode);
    // TODO: Properly handle return values.
    // POSIX errors are different from the SCE_ ones
    return ret;
}

}   // End namespace PS4::OS::Libs::Kernel