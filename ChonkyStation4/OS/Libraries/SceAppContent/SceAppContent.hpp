#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>


class Module;

namespace PS4::OS::Libs::SceAppContent {

void init(Module& module);

static constexpr s32 SCE_APP_CONTENT_MOUNTPOINT_DATA_MAXSIZE = 16;

using SceAppContentTemporaryDataOption = u32;

struct SceAppContentMountPoint {
    char data[SCE_APP_CONTENT_MOUNTPOINT_DATA_MAXSIZE];
};

struct SceAppContentAddcontInfo;

s32 PS4_FUNC sceAppContentTemporaryDataMount(SceAppContentTemporaryDataOption option, SceAppContentMountPoint* mount_point);
s32 PS4_FUNC sceAppContentTemporaryDataMount2(SceAppContentTemporaryDataOption option, SceAppContentMountPoint* mount_point);
s32 PS4_FUNC sceAppContentTemporaryDataGetAvailableSpaceKb(SceAppContentMountPoint* mount_point, size_t* available_space_kb);
s32 PS4_FUNC sceAppContentGetAddcontInfoList(Np::SceNpServiceLabel service_label, SceAppContentAddcontInfo* list, u32 n_list, u32* n_hit);

}   // End namespace PS4::OS::Libs::SceAppContent