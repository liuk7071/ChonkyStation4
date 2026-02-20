#include "SceNpManager.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/UserManagement.hpp>
#include <OS/Libraries/SceNpMatching/SceNpMatching.hpp>


namespace PS4::OS::Libs::SceNpManager {

MAKE_LOG_FUNCTION(log, lib_sceNpManager);

static constexpr s32 SCE_NP_POLL_ASYNC_RET_FINISHED = 0;
static constexpr s32 SCE_NP_POLL_ASYNC_RET_RUNNING  = 1;

void init(Module& module) {
    module.addSymbolExport("3Zl8BePTh9Y", "sceNpCheckCallback", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckCallback);
    module.addSymbolExport("eQH7nWPcAgc", "sceNpGetState", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetState);
    module.addSymbolExport("p-o74CnoNzY", "sceNpGetNpId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetNpId);
    module.addSymbolExport("XDncXQIJUSk", "sceNpGetOnlineId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetOnlineId);
    module.addSymbolExport("eiqMCt9UshI", "sceNpCreateAsyncRequest", "libSceNpManager", "libSceNpManager", (void*)&sceNpCreateAsyncRequest);
    module.addSymbolExport("uqcPJLWL08M", "sceNpPollAsync", "libSceNpManager", "libSceNpManager", (void*)&sceNpPollAsync);
    module.addSymbolExport("r6MyYJkryz8", "sceNpCheckPlus", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckPlus);
    module.addSymbolExport("ilwLM4zOmu4", "sceNpGetParentalControlInfo", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetParentalControlInfo);
    
    module.addSymbolStub("Ec63y59l9tw", "sceNpSetNpTitleId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A2CQ3kgSopQ", "sceNpSetContentRestriction", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("VfRSmPmj8Q8", "sceNpRegisterStateCallback", "libSceNpManager", "libSceNpManager", 0);
    module.addSymbolStub("qQJfO8HAiaY", "sceNpRegisterStateCallbackA", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("mjjTXh+NHWY", "sceNpUnregisterStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("xViqJdDgKl0", "sceNpUnregisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("hw5KNqAAels", "sceNpRegisterNpReachabilityStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("cRILAEvn+9M", "sceNpUnregisterNpReachabilityStateCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GImICnh+boA", "sceNpRegisterPlusEventCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("uFJpaKNBAj4", "sceNpRegisterGamePresenceCallback", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("KswxLxk4c1Y", "sceNpRegisterGamePresenceCallbackA", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GpLQDNKICac", "sceNpCreateRequest", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("2rsFmlGWleQ", "sceNpCheckNpAvailability", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("S7QTn72PrDw", "sceNpDeleteRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("OzKvTvg3ZYU", "sceNpAbortRequest", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("GFhVUpRmbHE", "sceNpInGameMessageInitialize", "libSceNpManager", "libSceNpManager");
    
    module.addSymbolStub("0c7HbXRKUt4", "sceNpRegisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("YIvqqvJyjEc", "sceNpUnregisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("JELHf4xPufo", "sceNpCheckCallbackForLib", "libSceNpManagerForToolkit", "libSceNpManager");
}

static bool is_signed_in = false;

s32 PS4_FUNC sceNpCheckCallback() {
    log("sceNpCheckCallback()\n");

    SceNpMatching::checkCallback();
    // TODO: Other callbacks
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state) {
    log("sceNpGetState(uid=%d, state=*%p)\n", uid, state);

    *state = is_signed_in ? SceNpState::SCE_NP_STATE_SIGNED_IN : SceNpState::SCE_NP_STATE_SIGNED_OUT;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetNpId(SceUserService::SceUserServiceUserId uid, SceNpId* np_id) {
    log("sceNpGetNpId(uid=%d, np_id=*%p)\n", uid, np_id);

    if (!is_signed_in)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Return dummy NpId
    std::memset(np_id, 0, sizeof(SceNpId));
    std::strcpy(np_id->handle.data, "ChonkyStation4");
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id) {
    log("sceNpGetOnlineId(uid=%d, online_id=*%p)\n", uid, online_id);

    if (!is_signed_in)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Return dummy OnlineId
    std::memset(online_id, 0, sizeof(SceNpOnlineId));
    std::strcpy(online_id->data, "ChonkyStation4");
    return SCE_OK;
}

s32 PS4_FUNC sceNpCreateRequest() {
    log("sceNpCreateRequest()\n");

    auto* req = OS::make<SceNpRequest>();
    req->is_async = false;
    return req->handle;
}

s32 PS4_FUNC sceNpCreateAsyncRequest(const SceNpCreateAsyncRequestParameter* param) {
    log("sceNpCreateAsyncRequest(param=*%p)\n", param);
    
    auto* req = OS::make<SceNpRequest>();
    req->is_async = true;
    req->state = SceNpRequest::State::Running;
    return req->handle;
}

s32 PS4_FUNC sceNpPollAsync(s32 req_id, s32* result) {
    log("sceNpPollAsync(req_id=%d, result=*%p)\n", req_id, result);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    if (!req->is_async) {
        Helpers::panic("sceNpPollAsync: specified request was not async\n");
    }

    const bool is_req_done = req->state == SceNpRequest::State::Finished;
    if (is_req_done) {
        *result = req->result;
    }

    return is_req_done ? SCE_NP_POLL_ASYNC_RET_FINISHED : SCE_NP_POLL_ASYNC_RET_RUNNING;
}

s32 PS4_FUNC sceNpCheckPlus(s32 req_id, const SceNpCheckPlusParameter* param, SceNpCheckPlusResult* result) {
    log("sceNpCheckPlus(req_id=%d, param=*%p, result=*%p)\n", req_id, param, result);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    result->authorized = true;

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetParentalControlInfo(s32 req_id, SceNpOnlineId* online_id, s8* age, SceNpParentalControlInfo* info) {
    log("sceNpGetParentalControlInfo(req_id=%d, online_id=\"%s\", age=*%p, info=*%p)\n", req_id, online_id->data, age, info);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    *age = 18;
    info->content_restriction   = false;
    info->chat_restriction      = false;
    info->ugc_restriction       = false;
    
    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager