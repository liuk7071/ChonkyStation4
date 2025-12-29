#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceSaveData {

void init(Module& module);

static constexpr s32 SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE = 16;

struct SceSaveDataMount2;
using SceSaveDataBlocks = u64;
using SceSaveDataMountStatus = u32;

struct SceSaveDataMountPoint {
    char data[SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE];
};

struct SceSaveDataMountResult {
    SceSaveDataMountPoint mount_point;
    SceSaveDataBlocks required_blocks;
    u32 unused;
    SceSaveDataMountStatus mount_status;
    u8 reserved[28];
    s32 : 32;
};

s32 PS4_FUNC sceSaveDataMount(const SceSaveDataMount2* mount, SceSaveDataMountResult* mount_result);

}   // End namespace PS4::OS::Libs::SceSaveData