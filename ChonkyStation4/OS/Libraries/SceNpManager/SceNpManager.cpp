#include "SceNpManager.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceNpManager {

MAKE_LOG_FUNCTION(log, lib_sceNpManager);

void init(Module& module) {
    module.addSymbolStub("eQH7nWPcAgc", "sceNpGetState", "libSceNpManager", "libSceNpManager");
    
    module.addSymbolStub("Ec63y59l9tw", "sceNpSetNpTitleId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A2CQ3kgSopQ", "sceNpSetContentRestriction", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("p-o74CnoNzY", "sceNpGetNpId", "libSceNpManager", "libSceNpManager", 0x8055000a /* user not signed up */);
    module.addSymbolStub("VfRSmPmj8Q8", "sceNpRegisterStateCallback", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("GImICnh+boA", "sceNpRegisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("3Zl8BePTh9Y", "sceNpCheckCallback", "libSceNpManager", "libSceNpManager");
}

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state) {
    log("sceNpGetState(uid=%d, state=*%p)\n", uid, state);

    *state = SceNpState::SCE_NP_STATE_SIGNED_OUT;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager