#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceZlib {

void init(Module& module);

static constexpr s32 SCE_ZLIB_ERROR_INVALID = 0x81120016;
static constexpr s32 SCE_ZLIB_ERROR_ALREADY_INITIALIZED = 0x81120033;

s32 PS4_FUNC sceZlibInitialize(const void* buffer, size_t length);
s32 PS4_FUNC sceZlibInflate(const void* src, u32 src_len, void* dst, u32 dst_len, u64* req_id);
s32 PS4_FUNC sceZlibWaitForDone(u64* req_id, u32* timeout);
s32 PS4_FUNC sceZlibGetResult(u64 req_id, u32* dst_len, s32* status);

}   // End namespace PS4::OS::Libs::SceZlib