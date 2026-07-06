#include "SceNpTrophy.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceNpTrophy {

MAKE_LOG_FUNCTION(log, lib_sceNpTrophy);

void init(Module& module) {
    module.addSymbolExport("XbkjbobZlCY", "sceNpTrophyCreateContext", "libSceNpTrophy", "libSceNpTrophy", (void*)&sceNpTrophyCreateContext);
    module.addSymbolExport("q7U6tEAQf7c", "sceNpTrophyCreateHandle", "libSceNpTrophy", "libSceNpTrophy", (void*)&sceNpTrophyCreateHandle);
    module.addSymbolExport("LHuSmO3SLd8", "sceNpTrophyGetTrophyUnlockState", "libSceNpTrophy", "libSceNpTrophy", (void*)&sceNpTrophyGetTrophyUnlockState);
    
    module.addSymbolStub("TJCAxto9SEU", "sceNpTrophyRegisterContext", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("qqUVGDgQBm0", "sceNpTrophyGetTrophyInfo", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("YYP3f2W09og", "sceNpTrophyGetGameInfo", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("28xmRUFao68", "sceNpTrophyUnlockTrophy", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("GNcF4oidY0Y", "sceNpTrophyDestroyHandle", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("aTnHs7W-9Uk", "sceNpTrophyAbortHandle", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("E1Wrwd07Lr8", "sceNpTrophyDestroyContext", "libSceNpTrophy", "libSceNpTrophy");
}

s32 PS4_FUNC sceNpTrophyCreateContext(SceNpTrophyContext* ctx, SceUserService::SceUserServiceUserId uid, Np::SceNpServiceLabel service_label, u64 options) {
    log("sceNpTrophyCreateContext(ctx=*%p, uid=%d, service_label=%d, options=%lld)\n", ctx, uid, service_label, options);

    *ctx = 100;
    return SCE_OK;
}

s32 PS4_FUNC sceNpTrophyCreateHandle(SceNpTrophyHandle* handle) {
    log("sceNpTrophyCreateHandle(handle=*%p)\n", handle);

    *handle = 100;
    return SCE_OK;
}

s32 PS4_FUNC sceNpTrophyGetTrophyUnlockState(SceNpTrophyContext ctx, SceNpTrophyHandle handle, SceNpTrophyFlagArray* flags, u32* count) {
    log("sceNpTrophyGetTrophyUnlockState(ctx=%d, handle=%d, flags=*%p, count=*%p)\n");

    // TODO
    *count = 0;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceSysmodule