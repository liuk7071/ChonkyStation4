#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>   // For SceUserServiceUserId


class Module;

namespace PS4::OS::Libs::SceSaveData {

void init(Module& module);

static constexpr s32 SCE_SAVE_DATA_ERROR_MEMORY_NOT_READY   = 0x809f0012;

static constexpr s32 SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE = 16;
static constexpr s32 SCE_SAVE_DATA_TITLE_ID_DATA_SIZE       = 10;
static constexpr s32 SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE     = 32;
static constexpr s32 SCE_SAVE_DATA_TITLE_MAXSIZE            = 128;
static constexpr s32 SCE_SAVE_DATA_SUBTITLE_MAXSIZE         = 128;
static constexpr s32 SCE_SAVE_DATA_DETAIL_MAXSIZE           = 1024;

struct SceSaveDataFingerprint;

using SceSaveDataBlocks = u64;
using SceSaveDataMountMode = u32;
using SceSaveDataMountStatus = u32;
using SceSaveDataSortKey = u32;
using SceSaveDataSortOrder = u32;

struct SceSaveDataMountPoint {
    char data[SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE];
};

struct SceSaveDataDirName {
    char data[SCE_SAVE_DATA_DIRNAME_DATA_MAXSIZE];
};

struct SceSaveDataTitleId {
    char data[SCE_SAVE_DATA_TITLE_ID_DATA_SIZE];
    u8 padding[6];
};

struct SceSaveDataParam {
    char title[SCE_SAVE_DATA_TITLE_MAXSIZE];
    char subtitle[SCE_SAVE_DATA_SUBTITLE_MAXSIZE];
    char detail[SCE_SAVE_DATA_DETAIL_MAXSIZE];
    u32 user_param;
    s32 : 32;
    time_t mtime;
    u8 reserved[32];
};

struct SceSaveDataSearchInfo {
    SceSaveDataBlocks blocks;
    SceSaveDataBlocks free_blocks;
    u8 reserved[32];
};

struct SceSaveDataMount {
    SceUserService::SceUserServiceUserId user_id;
    int : 32;
    const SceSaveDataTitleId* title_id;
    const SceSaveDataDirName* dir_name;
    const SceSaveDataFingerprint* fingerprint;
    SceSaveDataBlocks blocks;
    SceSaveDataMountMode mount_mode;
    u8 reserved[32];
    int : 32;
};

struct SceSaveDataMount2 {
    SceUserService::SceUserServiceUserId user_id;
    int : 32;
    const SceSaveDataDirName* dir_name;
    SceSaveDataBlocks blocks;
    SceSaveDataMountMode mount_mode;
    u8 reserved[32];
    int : 32;
};

struct SceSaveDataMountResult {
    SceSaveDataMountPoint mount_point;
    SceSaveDataBlocks required_blocks;
    u32 unused;
    SceSaveDataMountStatus mount_status;
    u8 reserved[28];
    s32 : 32;
};

struct SceSaveDataDirNameSearchCond {
    SceUserService::SceUserServiceUserId user_id;
    int : 32;
    const SceSaveDataTitleId* title_id;
    const SceSaveDataDirName* dir_name;
    SceSaveDataSortKey key;
    SceSaveDataSortOrder order;
    uint8_t reserved[32];
};

struct SceSaveDataDirNameSearchResult {
    u32 n_hits;
    s32 : 32;
    SceSaveDataDirName* dir_names;
    u32 n_dir_names;
    u32 n_dir_names_set;
    SceSaveDataParam* params;
    SceSaveDataSearchInfo* infos;
    u8 reserved[12];
    s32 : 32;
};

s32 PS4_FUNC sceSaveDataMount(const SceSaveDataMount* mount, SceSaveDataMountResult* mount_result);
s32 PS4_FUNC sceSaveDataMount2(const SceSaveDataMount2* mount, SceSaveDataMountResult* mount_result);
s32 PS4_FUNC sceSaveDataSetupSaveDataMemory(const SceUserService::SceUserServiceUserId user_id, const size_t memory_size, SceSaveDataParam* param);
s32 PS4_FUNC sceSaveDataGetSaveDataMemory(const SceUserService::SceUserServiceUserId user_id, void* buf, const size_t buf_size, const s64 offset);
s32 PS4_FUNC sceSaveDataSetSaveDataMemory(const SceUserService::SceUserServiceUserId user_id, void* buf, const size_t buf_size, const s64 offset);
s32 PS4_FUNC sceSaveDataDirNameSearch(const SceSaveDataDirNameSearchCond* cond, SceSaveDataDirNameSearchResult* result);
s32 PS4_FUNC sceSaveDataGetProgress(float* progress);

}   // End namespace PS4::OS::Libs::SceSaveData