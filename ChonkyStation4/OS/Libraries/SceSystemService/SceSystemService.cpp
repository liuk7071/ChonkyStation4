#include "SceSystemService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/UserManagement.hpp>


namespace PS4::OS::Libs::SceSystemService {

MAKE_LOG_FUNCTION(log, lib_sceSystemService);

void init(Module& module) {
    module.addSymbolExport("fZo48un7LK4", "sceSystemServiceParamGetInt", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceParamGetInt);
    module.addSymbolExport("SsC-m-S9JTA", "sceSystemServiceParamGetString", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceParamGetString);
    module.addSymbolExport("rPo6tV8D9bM", "sceSystemServiceGetStatus", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceGetStatus);
    module.addSymbolExport("1n37q1Bvc5Y", "sceSystemServiceGetDisplaySafeAreaInfo", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceGetDisplaySafeAreaInfo);
    
    module.addSymbolStub("Vo5V8KAwCmk", "sceSystemServiceHideSplashScreen", "libSceSystemService", "libSceSystemService");
    module.addSymbolStub("9kPCz7Or+1Y", "sceSystemServiceReenableMusicPlayer", "libSceSystemService", "libSceSystemService");
    module.addSymbolStub("x1UB9bwDSOw", "sceSystemServiceDisableMusicPlayer", "libSceSystemService", "libSceSystemService");
    module.addSymbolStub("jA629PcMCKU", "sceSystemServiceGetRenderingMode", "libSceSystemService", "libSceSystemService");
    module.addSymbolStub("SYqaqLuQU6w", "sceSystemServiceIsBgmPlaying", "libSceSystemService", "libSceSystemService");
    
    module.addSymbolStub("nT-7-iG55M8", "sceSystemServiceSetPowerSaveLevel", "libSceSystemServicePowerSaveLevel", "libSceSystemService");
    
    module.addSymbolStub("f-Q8Nd33FBc", "sceLncUtilInitialize", "libSceLncUtil", "libSceSystemService");
    module.addSymbolStub("awS+eYVuXJA", "sceLncUtilRegisterShellUI", "libSceLncUtil", "libSceSystemService");
    module.addSymbolStub("DxRki7T2E44", "sceLncUtilGetAppStatus", "libSceLncUtil", "libSceSystemService");
    module.addSymbolStub("cyO5ShJxdnE", "sceLncUtilGetAppStatusListForShellUIReboot", "libSceLncUtil", "libSceSystemService");
    module.addSymbolStub("v7DYuX0G5TQ", "sceLncUtilSetAppFocus", "libSceLncUtil", "libSceSystemService");
    module.addSymbolStub("X8gYbyLG1wk", "sceLncUtilSetControllerFocus", "libSceLncUtil", "libSceSystemService");
    module.addSymbolStub("ZucoOmNsb7w", "sceLncUtilGetEventForShellUI", "libSceLncUtil", "libSceSystemService", -1);

    module.addSymbolStub("9plZCCRm9x4", "sceShellCoreUtilEnterPowerLockSection", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("nENvUAsAKdY", "sceShellCoreUtilLeavePowerLockSection", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("WISL-JH-6Ic", "sceShellCoreUtilGetAppData", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("xKSgaSVX1io", "sceShellCoreUtilSetAppData", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("l96YlUEtMPk", "sceShellCoreUtilSetDeviceIndexBehavior", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("TJp3kdSGsIw", "sceShellCoreUtilSetImposeStatusFlag", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("plK52OfeEIc", "sceShellCoreUtilGetUserIdOfMorpheusUser", "libSceShellCoreUtil", "libSceSystemService", -1);   // PSVR
    module.addSymbolStub("fORZmlh1TQo", "sceShellCoreUtilGetUIStatus", "libSceShellCoreUtil", "libSceSystemService", -1);
    module.addSymbolStub("atiUTsTFJ3k", "sceShellCoreUtilSetUIStatus", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("CKTyfq2tb7k", "sceShellCoreUtilGetPlatformPrivacySetting", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("K33+EwitWlo", "sceShellCoreUtilSetGameLiveStreamingStatus", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("bC8vo608P2E", "sceShellCoreUtilSetGameLiveStreamingOnAirFlag", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("3JNHzrEDnrk", "sceShellCoreUtilIsPowerSaveAlertRequested", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("vzWoetyaUuA", "sceShellCoreUtilIsTemperatureDanger", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("GEZ9sIz3wuM", "sceShellCoreUtilIsShowCrashReport", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("lAvSrKAjxCA", "sceShellCoreUtilGetBasicProductShape", "libSceShellCoreUtil", "libSceSystemService");
    module.addSymbolStub("mpeGML7ulA8", "sceShellCoreUtilIsExternalStorageAppMoveInProgress", "libSceShellCoreUtil", "libSceSystemService");
    
    module.addSymbolStub("ze0ky5Q1yE8", "sceSystemStateMgrGetCurrentState", "libSceSystemStateMgr", "libSceSystemService");
    module.addSymbolStub("wlxvESTUplk", "sceSystemStateMgrGetTriggerCode", "libSceSystemStateMgr", "libSceSystemService");

    module.addSymbolStub("ZVRXXqj1n80", "sceAppMessagingTryReceiveMsg", "libSceAppMessaging", "libSceSystemService", -1 /* don't know the correct error for "no message" */);
}

s32 PS4_FUNC sceSystemServiceParamGetInt(SceSystemServiceParamId param_id, s32* val) {
    log("sceSystemServiceParamGetInt(param_id=%d, value=*%p)\n", param_id, val);

    s32 ret = 0;
    switch (param_id) {
    case SCE_SYSTEM_SERVICE_PARAM_ID_LANG:                  ret = 1;    break;  // English (United States)
    case SCE_SYSTEM_SERVICE_PARAM_ID_DATE_FORMAT:           ret = 1;    break;  // DD/MM/YYYY
    case SCE_SYSTEM_SERVICE_PARAM_ID_ENTER_BUTTON_ASSIGN:   ret = 1;    break;  // Cross

    default:
        Helpers::panic("sceSystemServiceParamGetInt: unhandled param_id=%d\n", param_id);
    }

    *val = ret;
    return SCE_OK;
}

s32 PS4_FUNC sceSystemServiceParamGetString(SceSystemServiceParamId param_id, char* buf, size_t buf_size) {
    log("sceSystemServiceParamGetString(param_id=%d, buf=*%p, buf_size=%lld)\n", param_id, buf, buf_size);

    switch (param_id) {
    case SCE_SYSTEM_SERVICE_PARAM_ID_SYSTEM_NAME:   std::strncpy(buf, "PS4", buf_size);     break;

    default:
        Helpers::panic("sceSystemServiceParamGetString: unhandled param_id=%d\n", param_id);
    }

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