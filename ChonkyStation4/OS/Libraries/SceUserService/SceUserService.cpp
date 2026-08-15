#include "SceUserService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/UserManagement.hpp>


namespace PS4::OS::Libs::SceUserService {

MAKE_LOG_FUNCTION(log, lib_sceUserService);

void init(Module& module) {
    module.addSymbolExport("yH17Q6NWtVg", "sceUserServiceGetEvent", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetEvent);
    module.addSymbolExport("CdWp0oHWGr0", "sceUserServiceGetInitialUser", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetInitialUser);
    module.addSymbolExport("fPhymKNvK-A", "sceUserServiceGetLoginUserIdList", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetLoginUserIdList);
    module.addSymbolExport("1xxcMiGu2fo", "sceUserServiceGetUserName", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetUserName);
    module.addSymbolExport("J-KEr4gUEvQ", "sceUserServiceGetHomeDirectory", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetHomeDirectory);
    module.addSymbolExport("x6m8P9DBPSc", "sceUserServiceGetThemeEntitlementId", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetThemeEntitlementId);
    module.addSymbolExport("eNb53LQJmIM", "sceUserServiceGetForegroundUser", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetForegroundUser);
    
    module.addSymbolStub("j3YMu1MVNNo", "sceUserServiceInitialize", "libSceUserService", "libSceUserService");
    module.addSymbolStub("az-0R6eviZ0", "sceUserServiceInitialize2", "libSceUserService", "libSceUserService");
    module.addSymbolStub("lUoqwTQu4Go", "sceUserServiceGetUserColor", "libSceUserService", "libSceUserService");
    module.addSymbolStub("wuI7c7UNk0A", "sceUserServiceRegisterEventCallback", "libSceUserService", "libSceUserService");
    module.addSymbolStub("5EiQCnL2G1Y", "sceUserServiceGetRegisteredUserIdList", "libSceUserService", "libSceUserService");
    module.addSymbolStub("WGXOvoUwrOs", "sceUserServiceGetCreatedVersion", "libSceUserService", "libSceUserService");
    module.addSymbolStub("6dfDreosXGY", "sceUserServiceGetNpAccountId", "libSceUserService", "libSceUserService");
    module.addSymbolStub("fEy0EW0AR18", "sceUserServiceGetNpOfflineAccountId", "libSceUserService", "libSceUserService");
    module.addSymbolStub("FnWkLNOmJXw", "sceUserServiceIsGuestUser", "libSceUserService", "libSceUserService");

    module.addSymbolStub("rnEhHqG-4xo", "sceUserServiceGetAccessibilityChatTranscription", "libSceUserService", "libSceUserService");
    module.addSymbolStub("g6ojqW3c8Z4", "sceUserServiceGetAccessibilityKeyremapData", "libSceUserService", "libSceUserService");
    module.addSymbolStub("xrtki9sUopg", "sceUserServiceGetAccessibilityKeyremapEnable", "libSceUserService", "libSceUserService");
    module.addSymbolStub("ZKJtxdgvzwg", "sceUserServiceGetAccessibilityPressAndHoldDelay", "libSceUserService", "libSceUserService");
    module.addSymbolStub("-3Y5GO+-i78", "sceUserServiceGetAccessibilityTriggerEffect", "libSceUserService", "libSceUserService");
    module.addSymbolStub("qWYHOFwqCxY", "sceUserServiceGetAccessibilityVibration", "libSceUserService", "libSceUserService");
    module.addSymbolStub("1zDEFUmBdoo", "sceUserServiceGetAccessibilityZoom", "libSceUserService", "libSceUserService");
    module.addSymbolStub("hD-H81EN9Vg", "sceUserServiceGetAccessibilityZoomEnabled", "libSceUserService", "libSceUserService");

    module.addSymbolStub("64PEUYPuK98", "sceUserServiceGetGlsAccessTokenNiconicoLive", "libSceUserService", "libSceUserService");
    module.addSymbolStub("8Y+aDvVGLiw", "sceUserServiceGetGlsAccessTokenTwitch", "libSceUserService", "libSceUserService");
    module.addSymbolStub("V7ZG7V+dd08", "sceUserServiceGetGlsAccessTokenUstream", "libSceUserService", "libSceUserService");
    module.addSymbolStub("QqZ1A3vukFM", "sceUserServiceGetGlsAnonymousUserId", "libSceUserService", "libSceUserService");
    module.addSymbolStub("FP4TKrdRXXM", "sceUserServiceGetGlsBcTags", "libSceUserService", "libSceUserService");
    module.addSymbolStub("yX-TpbFAYxo", "sceUserServiceGetGlsBcTitle", "libSceUserService", "libSceUserService");
    module.addSymbolStub("Mm4+PSflHbM", "sceUserServiceGetGlsBroadcastChannel", "libSceUserService", "libSceUserService");
    module.addSymbolStub("NpEYVDOyjRk", "sceUserServiceGetGlsBroadcastersComment", "libSceUserService", "libSceUserService");
    module.addSymbolStub("WvM21J1SI0U", "sceUserServiceGetGlsBroadcastersCommentColor", "libSceUserService", "libSceUserService");
    module.addSymbolStub("HxNRiCWfVFw", "sceUserServiceGetGlsBroadcastService", "libSceUserService", "libSceUserService");
    module.addSymbolStub("6ZQ4kfhM37c", "sceUserServiceGetGlsBroadcastUiLayout", "libSceUserService", "libSceUserService");
    module.addSymbolStub("YmmFiEoegko", "sceUserServiceGetGlsCamCrop", "libSceUserService", "libSceUserService");
    module.addSymbolStub("Y5U66nk0bUc", "sceUserServiceGetGlsCameraBgFilter", "libSceUserService", "libSceUserService");
    module.addSymbolStub("LbQ-jU9jOsk", "sceUserServiceGetGlsCameraBrightness", "libSceUserService", "libSceUserService");
    module.addSymbolStub("91kOKRnkrhE", "sceUserServiceGetGlsCameraChromaKeyLevel", "libSceUserService", "libSceUserService");
    module.addSymbolStub("1ppzHkQhiNs", "sceUserServiceGetGlsCameraContrast", "libSceUserService", "libSceUserService");
    module.addSymbolStub("jIe8ZED06XI", "sceUserServiceGetGlsCameraDepthLevel", "libSceUserService", "libSceUserService");
    module.addSymbolStub("0H51EFxR3mc", "sceUserServiceGetGlsCameraEdgeLevel", "libSceUserService", "libSceUserService");
    module.addSymbolStub("rLEw4n5yI40", "sceUserServiceGetGlsCameraEffect", "libSceUserService", "libSceUserService");
    module.addSymbolStub("+Prbx5iagl0", "sceUserServiceGetGlsCameraEliminationLevel", "libSceUserService", "libSceUserService");
    module.addSymbolStub("F0wuEvioQd4", "sceUserServiceGetGlsCameraPosition", "libSceUserService", "libSceUserService");
    module.addSymbolStub("GkcHilidQHk", "sceUserServiceGetGlsCameraReflection", "libSceUserService", "libSceUserService");
    module.addSymbolStub("zBLxX8JRMoo", "sceUserServiceGetGlsCameraSize", "libSceUserService", "libSceUserService");
    module.addSymbolStub("O1nURsxyYmk", "sceUserServiceGetGlsCameraTransparency", "libSceUserService", "libSceUserService");
    module.addSymbolStub("4TOEFdmFVcI", "sceUserServiceGetGlsCommunityId", "libSceUserService", "libSceUserService");
    module.addSymbolStub("+29DSndZ9Oc", "sceUserServiceGetGlsFloatingMessage", "libSceUserService", "libSceUserService");
    module.addSymbolStub("ki81gh1yZDM", "sceUserServiceGetGlsHintFlag", "libSceUserService", "libSceUserService");
    module.addSymbolStub("zR+J2PPJgSU", "sceUserServiceGetGlsInitSpectating", "libSceUserService", "libSceUserService");
    module.addSymbolStub("8IqdtMmc5Uc", "sceUserServiceGetGlsIsCameraHidden", "libSceUserService", "libSceUserService");
    module.addSymbolStub("f5lAVp0sFNo", "sceUserServiceGetGlsIsFacebookEnabled", "libSceUserService", "libSceUserService");
    module.addSymbolStub("W3neFYAvZss", "sceUserServiceGetGlsIsMuteEnabled", "libSceUserService", "libSceUserService");
    module.addSymbolStub("4IXuUaBxzEg", "sceUserServiceGetGlsIsRecDisabled", "libSceUserService", "libSceUserService");
    module.addSymbolStub("hyW5w855fk4", "sceUserServiceGetGlsIsRecievedMessageHidden", "libSceUserService", "libSceUserService");
    module.addSymbolStub("Xp9Px0V0tas", "sceUserServiceGetGlsIsTwitterEnabled", "libSceUserService", "libSceUserService");
    module.addSymbolStub("uMkqgm70thg", "sceUserServiceGetGlsLanguageFilter", "libSceUserService", "libSceUserService");
    module.addSymbolStub("LyXzCtzleAQ", "sceUserServiceGetGlsLfpsSortOrder", "libSceUserService", "libSceUserService");
    module.addSymbolStub("CvwCMJtzp1I", "sceUserServiceGetGlsLiveQuality", "libSceUserService", "libSceUserService");
    module.addSymbolStub("Z+dzNaClq7w", "sceUserServiceGetGlsLiveQuality2", "libSceUserService", "libSceUserService");
    module.addSymbolStub("X5On-7hVCs0", "sceUserServiceGetGlsLiveQuality3", "libSceUserService", "libSceUserService");
    module.addSymbolStub("+qAE4tRMrXk", "sceUserServiceGetGlsLiveQuality4", "libSceUserService", "libSceUserService");
    module.addSymbolStub("4ys00CRU6V8", "sceUserServiceGetGlsLiveQuality5", "libSceUserService", "libSceUserService");
    module.addSymbolStub("75cwn1y2ffk", "sceUserServiceGetGlsMessageFilterLevel", "libSceUserService", "libSceUserService");
    module.addSymbolStub("2-MkHLDkFP4", "sceUserServiceGetGlsOverlayPosition", "libSceUserService", "libSceUserService");
    module.addSymbolStub("pAcXoWY-JV8", "sceUserServiceGetGlsSortOrder", "libSceUserService", "libSceUserService");
    module.addSymbolStub("EeVJ6rikwss", "sceUserServiceGetGlsSortOrderGame", "libSceUserService", "libSceUserService");
    module.addSymbolStub("FjbOtABSsKU", "sceUserServiceGetGlsStreamingMode", "libSceUserService", "libSceUserService");
    module.addSymbolStub("+NVJMeISrM4", "sceUserServiceGetGlsTtsFlags", "libSceUserService", "libSceUserService");
    module.addSymbolStub("eQrBbMmZ1Ss", "sceUserServiceGetGlsTtsPitch", "libSceUserService", "libSceUserService");
    module.addSymbolStub("BCDA6jn4HVY", "sceUserServiceGetGlsTtsSpeed", "libSceUserService", "libSceUserService");
    module.addSymbolStub("SBurFYk7M74", "sceUserServiceGetGlsTtsVolume", "libSceUserService", "libSceUserService");

    //module.addSymbolStub("bwFjS+bX9mA", "sceUserServiceTerminate", "libSceUserService", "libSceUserService");

    module.addSymbolStub("wuI7c7UNk0A", "sceUserServiceRegisterEventCallback", "libSceUserServiceForNpToolkit", "libSceUserService");
}

bool is_logged_in = false;

s32 PS4_FUNC sceUserServiceGetEvent(SceUserServiceEvent* event) {
    log("sceUserServiceGetEvent(event=*%p)\n", event);
    
    // Send a login event
    if (!is_logged_in) {
        is_logged_in = true;
        event->event = SceUserServiceEventType::Login;
        event->user_id = User::current->getID();
        return SCE_OK;
    }

    return SCE_USER_SERVICE_ERROR_NO_EVENT;
}

s32 PS4_FUNC sceUserServiceGetInitialUser(SceUserServiceUserId* user_id) {
    log("sceUserServiceGetInitialUser(user_id=*%p)\n", user_id);

    *user_id = User::current->getID();
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetLoginUserIdList(SceUserServiceLoginUserIdList* user_id_list) {
    log("sceUserServiceGetLoginUserIdList(user_id_list=*%p)\n", user_id_list);

    for (int i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; i++)
        user_id_list->user_ids[i] = SCE_USER_SERVICE_USER_ID_INVALID;

    // Return 1 logged in user
    user_id_list->user_ids[0] = User::current->getID();
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetUserName(const SceUserServiceUserId user_id, char* username, const size_t size) {
    log("sceUserServiceGetUserName(user_id=%d, username=*%p, size=%lld)\n", user_id, username, size);

    const std::string name = User::current->getUsername();
    if (size < name.size()) return SCE_USER_SERVICE_ERROR_BUFFER_TOO_SHORT;

    std::memset(username, '\0', size);
    std::strcpy(username, name.c_str());
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetHomeDirectory(const SceUserServiceUserId user_id, char* out_dir) {
    log("sceUserServiceGetHomeDirectory(user_id=%d, out_dir=*%p)\n", user_id, out_dir);

    // TODO: Do it properly, this is not the real path
    std::strncpy(out_dir, "/system/home", 0x20);
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetThemeEntitlementId(const SceUserServiceUserId user_id, char* out_id, const size_t size) {
    log("sceUserServiceGetThemeEntitlementId(user_id=%d, out_id=*%p, size=%lld)\n", user_id, out_id, size);

    std::memset(out_id, '\0', size);
    std::strcpy(out_id, "aaaa-bbbb-cccc-dddd");
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetForegroundUser(SceUserServiceUserId* user_id) {
    log("sceUserServiceGetForegroundUser(user_id=*%p)\n", user_id);

    //*user_id = User::current->getID();
    *user_id = 0x10000000;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager