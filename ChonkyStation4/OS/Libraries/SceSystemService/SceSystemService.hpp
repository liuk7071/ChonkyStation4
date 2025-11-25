#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceSystemService {

void init(Module& module);

static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_LANG                   = 1;
static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_DATE_FORMAT            = 2;
static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_TIME_FORMAT            = 3;
static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_TIME_ZONE              = 4;
static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_SUMMERTIME             = 5;
static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_GAME_PARENTAL_LEVEL    = 7;
static constexpr s32 SCE_SYSTEM_SERVICE_PARAM_ID_ENTER_BUTTON_ASSIGN    = 1000;
 


using SceSystemServiceParamId = s32;

s32 PS4_FUNC sceSystemServiceParamGetInt(SceSystemServiceParamId param_id, s32* val);

}   // End namespace PS4::OS::Libs::SceSystemService