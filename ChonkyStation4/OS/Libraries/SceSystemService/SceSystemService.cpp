#include "SceSystemService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSystemService {

MAKE_LOG_FUNCTION(log, lib_sceSystemService);

void init(Module& module) {
    module.addSymbolExport("fZo48un7LK4", "sceSystemServiceParamGetInt", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceParamGetInt);
    module.addSymbolExport("rPo6tV8D9bM", "sceSystemServiceGetStatus", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceGetStatus);
    module.addSymbolExport("1n37q1Bvc5Y", "sceSystemServiceGetDisplaySafeAreaInfo", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceGetDisplaySafeAreaInfo);
    
    module.addSymbolStub("Vo5V8KAwCmk", "sceSystemServiceHideSplashScreen", "libSceSystemService", "libSceSystemService");
}

s32 PS4_FUNC sceSystemServiceParamGetInt(SceSystemServiceParamId param_id, s32* val) {
    log("sceSystemServiceParamGetInt(param_id=%d, value=*%p)\n", param_id, val);

    s32 ret = 0;
    switch (param_id) {
    case SCE_SYSTEM_SERVICE_PARAM_ID_LANG:                  ret = 1;    break;  // English (United States)
    case SCE_SYSTEM_SERVICE_PARAM_ID_ENTER_BUTTON_ASSIGN:   ret = 1;    break;  // Cross

    default:
        Helpers::panic("sceSystemServiceParamGetInt: unhandled param_id=%d\n", param_id);
    }

    *val = ret;
    return SCE_OK;
}

s32 PS4_FUNC sceSystemServiceGetStatus(SceSystemServiceStatus* status) {
    log("sceSystemServiceGetStatus(status=*%p)\n", status);

    status->n_events = 0;
    status->is_system_ui_overlaid = false;
    status->is_in_background_execution = false;
    status->is_cpu_mode_7_cpu_normal = true;
    status->is_game_live_streaming_on_air = false;
    status->is_out_of_vr_play_area = false;
    return SCE_OK;
}

s32 PS4_FUNC sceSystemServiceGetDisplaySafeAreaInfo(SceSystemServiceDisplaySafeAreaInfo* info) {
    log("sceSystemServiceGetDisplaySafeAreaInfo(info=%p)\n", info);
    
    info->ratio = 1.0f;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager