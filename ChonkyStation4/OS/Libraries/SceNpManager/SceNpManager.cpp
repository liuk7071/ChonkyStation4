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
    module.addSymbolStub("qQJfO8HAiaY", "sceNpRegisterStateCallbackA", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("mjjTXh+NHWY", "sceNpUnregisterStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xViqJdDgKl0", "sceNpUnregisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("hw5KNqAAels", "sceNpRegisterNpReachabilityStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("cRILAEvn+9M", "sceNpUnregisterNpReachabilityStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GImICnh+boA", "sceNpRegisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3Zl8BePTh9Y", "sceNpCheckCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("r6MyYJkryz8", "sceNpCheckPlus", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uFJpaKNBAj4", "sceNpRegisterGamePresenceCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("KswxLxk4c1Y", "sceNpRegisterGamePresenceCallbackA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GpLQDNKICac", "sceNpCreateRequest", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("eiqMCt9UshI", "sceNpCreateAsyncRequest", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("2rsFmlGWleQ", "sceNpCheckNpAvailability", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("ilwLM4zOmu4", "sceNpGetParentalControlInfo", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uqcPJLWL08M", "sceNpPollAsync", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("S7QTn72PrDw", "sceNpDeleteRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GFhVUpRmbHE", "sceNpInGameMessageInitialize", "libSceNpManager", "libSceNpManager");
    
    module.addSymbolStub("0c7HbXRKUt4", "sceNpRegisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("YIvqqvJyjEc", "sceNpUnregisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("JELHf4xPufo", "sceNpCheckCallbackForLib", "libSceNpManagerForToolkit", "libSceNpManager");
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