#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Libraries/SceRtc/SceRtc.hpp>
#include <OS/Np/NpTypes.hpp>


class Module;

namespace PS4::OS::Libs::SceNpTrophy {

void init(Module& module);

static constexpr s32 SCE_NP_TROPHY_GAME_TITLE_MAX_SIZE  = 128;
static constexpr s32 SCE_NP_TROPHY_GAME_DESCR_MAX_SIZE  = 1024;
static constexpr s32 SCE_NP_TROPHY_NAME_MAX_SIZE        = 128;
static constexpr s32 SCE_NP_TROPHY_DESCR_MAX_SIZE       = 1024;

using SceNpTrophyContext = s32;
using SceNpTrophyHandle = s32;
using SceNpTrophyId = s32;
using SceNpTrophyGrade = s32;
using SceNpTrophyGroupId = s32;
struct SceNpTrophyFlagArray;

struct SceNpTrophyGameDetails {
    size_t size;
    u32 num_groups;
    u32 num_trophies;
    u32 num_platinum;
    u32 num_gold;
    u32 num_silver;
    u32 num_bronze;
    char title[SCE_NP_TROPHY_GAME_TITLE_MAX_SIZE];
    char description[SCE_NP_TROPHY_GAME_DESCR_MAX_SIZE];
};

struct SceNpTrophyGameData {
    size_t size;
    u32 unlocked_trophies;
    u32 unlocked_platinum;
    u32 unlocked_gold;
    u32 unlocked_silver;
    u32 unlocked_bronze;
    u32 progress_percentage;
};

struct SceNpTrophyDetails {
    size_t size;
    SceNpTrophyId trophy_id;
    SceNpTrophyGrade trophy_grade;
    SceNpTrophyGroupId group_id;
    bool hidden;
    u8 reserved[3];
    char name[SCE_NP_TROPHY_NAME_MAX_SIZE];
    char description[SCE_NP_TROPHY_DESCR_MAX_SIZE];
};

struct SceNpTrophyData {
    size_t size;
    SceNpTrophyId trophy_id;
    bool unlocked;
    u8 reserved[3];
    SceRtc::SceRtcTick timestamp;
};

s32 PS4_FUNC sceNpTrophyCreateContext(SceNpTrophyContext* ctx, SceUserService::SceUserServiceUserId uid, Np::SceNpServiceLabel service_label, u64 options);
s32 PS4_FUNC sceNpTrophyCreateHandle(SceNpTrophyHandle* handle);
s32 PS4_FUNC sceNpTrophyGetTrophyUnlockState(SceNpTrophyContext ctx, SceNpTrophyHandle handle, SceNpTrophyFlagArray* flags, u32* count);
s32 PS4_FUNC sceNpTrophyGetGameInfo(SceNpTrophyContext ctx, SceNpTrophyHandle handle, SceNpTrophyGameDetails* details, SceNpTrophyGameData* data);
s32 PS4_FUNC sceNpTrophyGetTrophyInfo(SceNpTrophyContext ctx, SceNpTrophyHandle handle, SceNpTrophyId trophy_id, SceNpTrophyDetails* details, SceNpTrophyData* data);

}   // End namespace PS4::OS::Libs::SceNpTrophy