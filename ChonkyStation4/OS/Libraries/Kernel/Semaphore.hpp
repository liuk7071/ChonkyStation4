#pragma once

#include <Common.hpp>
#include <semaphore>
#include <memory>


namespace PS4::OS::Libs::Kernel {

using sema = std::counting_semaphore<UINT32_MAX>;

struct Semaphore {
    Semaphore(s32 init_count, s32 max_count) : max_count(max_count) {
        std_sema = std::make_unique<sema>(init_count);
    }

    std::string name;
    s32 max_count;
    std::unique_ptr<sema> std_sema;
};

struct SceKernelSemaOptParam;
using SceKernelSema = Semaphore*;

s32 PS4_FUNC sceKernelCreateSema(SceKernelSema* sem, const char* name, u32 attr, s32 init_count, s32 max_count, const SceKernelSemaOptParam* opt_param);

};  // End namespace PS4::OS::Libs::Kernel