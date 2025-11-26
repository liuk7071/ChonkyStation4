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

struct SceSystemServiceStatus {
    s32 n_events;
    bool is_system_ui_overlaid;
    bool is_in_background_execution;
    bool is_cpu_mode_7_cpu_normal;
    bool is_game_live_streaming_on_air;
    bool is_out_of_vr_play_area;
    u8 reserved[];
};

s32 PS4_FUNC sceSystemServiceParamGetInt(SceSystemServiceParamId param_id, s32* val);
s32 PS4_FUNC sceSystemServiceGetStatus(SceSystemServiceStatus* status);

}   // End namespace PS4::OS::Libs::SceSystemService