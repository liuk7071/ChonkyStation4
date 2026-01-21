#include "SceNpManager.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceNpManager {

MAKE_LOG_FUNCTION(log, lib_sceNpManager);

void init(Module& module) {
    module.addSymbolExport("eQH7nWPcAgc", "sceNpGetState", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetState);
    module.addSymbolExport("XDncXQIJUSk", "sceNpGetOnlineId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetOnlineId);
    
    module.addSymbolStub("Ec63y59l9tw", "sceNpSetNpTitleId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A2CQ3kgSopQ", "sceNpSetContentRestriction", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("p-o74CnoNzY", "sceNpGetNpId", "libSceNpManager", "libSceNpManager", 0x8055000a /* user not signed up */);
    module.addSymbolStub("VfRSmPmj8Q8", "sceNpRegisterStateCallback", "libSceNpManager", "libSceNpManager", 0);
    module.addSymbolStub("mjjTXh+NHWY", "sceNpUnregisterStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GImICnh+boA", "sceNpRegisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3Zl8BePTh9Y", "sceNpCheckCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uFJpaKNBAj4", "sceNpRegisterGamePresenceCallback", "libSceNpManager", "libSceNpManager");
}

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state) {
    log("sceNpGetState(uid=%d, state=*%p)\n", uid, state);

    *state = SceNpState::SCE_NP_STATE_SIGNED_OUT;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id) {
    log("sceNpGetOnlineId(uid=%d, online_id=*%p)\n", uid, online_id);
    return SCE_NP_ERROR_SIGNED_OUT;
}

}   // End namespace PS4::OS::Libs::SceNpManager