#pragma once

#include <Common.hpp>
#include <deque>


namespace PS4::OS::Libs::Kernel {

s32 PS4_FUNC kernel_open(const char* path, s32 flags, u16 mode);
s32 PS4_FUNC sceKernelOpen(const char* path, s32 flags, u16 mode);

};  // End namespace PS4::OS::Libs::Kernel