#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceRegMgr {

void init(Module& module);

static constexpr s32 SCE_REGMGR_ENT_KEY_SYSTEM_initialize           = 0x2040000;
static constexpr s32 SCE_REGMGR_ENT_KEY_SYSTEM_language             = 0x2020000;
static constexpr s32 SCE_REGMGR_ENT_KEY_SYSTEM_button_assign        = 0x20B0000;
static constexpr s32 SCE_REGMGR_ENT_KEY_VIDEOOUT_reset_reso_flag    = 0xA130000;

s32 PS4_FUNC sceRegMgrGetInt(s32 key, s32* data);
s32 PS4_FUNC sceRegMgrNonSysGetInt(s64 key, s32 unk, s32* data);

}   // End namespace PS4::OS::Libs::SceRegMgr