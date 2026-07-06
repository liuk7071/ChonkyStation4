#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Np/NpTypes.hpp>


class Module;

namespace PS4::OS::Libs::SceNpTrophy {

void init(Module& module);

using SceNpTrophyContext = s32;
using SceNpTrophyHandle = s32;
struct SceNpTrophyFlagArray;

s32 PS4_FUNC sceNpTrophyCreateContext(SceNpTrophyContext* ctx, SceUserService::SceUserServiceUserId uid, Np::SceNpServiceLabel service_label, u64 options);
s32 PS4_FUNC sceNpTrophyCreateHandle(SceNpTrophyHandle* handle);
s32 PS4_FUNC sceNpTrophyGetTrophyUnlockState(SceNpTrophyContext ctx, SceNpTrophyHandle handle, SceNpTrophyFlagArray* flags, u32* count);

}   // End namespace PS4::OS::Libs::SceNpTrophy