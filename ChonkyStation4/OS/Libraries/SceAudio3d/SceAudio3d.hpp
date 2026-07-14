#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceAudio3d {

void init(Module& module);

static constexpr s32 SCE_AUDIO3D_ERROR_NOT_READY = 0x80EA0007;

using SceAudio3dPortId = u32;

enum SceAudio3dBlocking {
    SCE_AUDIO3D_BLOCKING_ASYNC = 0,
    SCE_AUDIO3D_BLOCKING_SYNC = 1
};

s32 PS4_FUNC sceAudio3dPortPush(SceAudio3dPortId port_id, SceAudio3dBlocking blocking);
s32 PS4_FUNC sceAudio3dPortAdvance(SceAudio3dPortId port_id);
s32 PS4_FUNC sceAudio3dPortGetQueueLevel(SceAudio3dPortId port_id, u32* pending, u32* available);

}   // End namespace PS4::OS::Libs::SceAudio3d