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
    module.addSymbolExport("Oad3rvY-NJQ", "sceNpHasSignedUp", "libSceNpManager", "libSceNpManager", (void*)&sceNpHasSignedUp);
    module.addSymbolExport("rbknaUjpqWo", "sceNpGetAccountIdA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetAccountIdA);
    module.addSymbolExport("XDncXQIJUSk", "sceNpGetOnlineId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetOnlineId);
    module.addSymbolExport("VgYczPGB5ss", "sceNpGetUserIdByAccountId", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetUserIdByAccountId);
    module.addSymbolExport("JT+t00a3TxA", "sceNpGetAccountCountryA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetAccountCountryA);
    module.addSymbolExport("eiqMCt9UshI", "sceNpCreateAsyncRequest", "libSceNpManager", "libSceNpManager", (void*)&sceNpCreateAsyncRequest);
    module.addSymbolExport("uqcPJLWL08M", "sceNpPollAsync", "libSceNpManager", "libSceNpManager", (void*)&sceNpPollAsync);
    module.addSymbolExport("r6MyYJkryz8", "sceNpCheckPlus", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckPlus);
    module.addSymbolExport("ilwLM4zOmu4", "sceNpGetParentalControlInfo", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetParentalControlInfo);
    module.addSymbolExport("m9L3O6yst-U", "sceNpGetParentalControlInfoA", "libSceNpManager", "libSceNpManager", (void*)&sceNpGetParentalControlInfoA);
    module.addSymbolExport("8Z2Jc5GvGDI", "sceNpCheckNpAvailabilityA", "libSceNpManager", "libSceNpManager", (void*)&sceNpCheckNpAvailabilityA);
    
    module.addSymbolStub("Ec63y59l9tw", "sceNpSetNpTitleId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A2CQ3kgSopQ", "sceNpSetContentRestriction", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("VfRSmPmj8Q8", "sceNpRegisterStateCallback", "libSceNpManager", "libSceNpManager", 0);
    module.addSymbolStub("qQJfO8HAiaY", "sceNpRegisterStateCallbackA", "libSceNpManager", "libSceNpManager", 1);
    module.addSymbolStub("oPO9U42YpgI", "sceNpGetGamePresenceStatusA", "libSceNpManager", "libSceNpManager");
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
    module.addSymbolStub("IPb1hd1wAGc", "sceNpGetGamePresenceStatus", "libSceNpManager", "libSceNpManager");
    
    module.addSymbolStub("0c7HbXRKUt4", "sceNpRegisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("YIvqqvJyjEc", "sceNpUnregisterStateCallbackForToolkit", "libSceNpManagerForToolkit", "libSceNpManager");
    module.addSymbolStub("JELHf4xPufo", "sceNpCheckCallbackForLib", "libSceNpManagerForToolkit", "libSceNpManager");
}

s32 PS4_FUNC sceNpCheckCallback() {
    log("sceNpCheckCallback()\n");

    SceNpMatching::checkCallback();
    // TODO: Other callbacks
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state) {
    log("sceNpGetState(uid=%d, state=*%p)\n", uid, state);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    *state = user->is_logged_in_psn ? SceNpState::SCE_NP_STATE_SIGNED_IN : SceNpState::SCE_NP_STATE_SIGNED_OUT;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetNpId(SceUserService::SceUserServiceUserId uid, SceNpId* np_id) {
    log("sceNpGetNpId(uid=%d, np_id=*%p)\n", uid, np_id);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Return dummy NpId
    std::memset(np_id, 0, sizeof(SceNpId));
    std::strcpy(np_id->handle.data, user->online_id.c_str());
    return SCE_OK;
}

s32 PS4_FUNC sceNpHasSignedUp(SceUserService::SceUserServiceUserId uid, bool* has_signed_up) {
    log("sceNpHasSignedUp(uid=%d, has_signed_up=*%p)\n", uid, has_signed_up);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    *has_signed_up = user->is_logged_in_psn;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetAccountIdA(SceUserService::SceUserServiceUserId uid, SceNpAccountId* account_id) {
    log("sceNpGetAccountIdA(uid=%d, account_id=*%p)\n", uid, account_id);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Return dummy NpAccountId
    *account_id = user->account_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id) {
    log("sceNpGetOnlineId(uid=%d, online_id=*%p)\n", uid, online_id);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    std::memset(online_id, 0, sizeof(SceNpOnlineId));
    std::strcpy(online_id->data, user->online_id.c_str());
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetUserIdByAccountId(SceNpAccountId account_id, SceUserService::SceUserServiceUserId* uid) {
    log("sceNpGetUserIdByAccountId(account_id=%d, uid=*%p)\n", account_id, uid);

    // TODO
    *uid = 1;
    return SCE_OK;
}

s32 PS4_FUNC sceNpGetAccountCountryA(SceUserService::SceUserServiceUserId uid, SceNpCountryCode* country_code) {
    log("sceNpGetAccountCountryA(uid=%d, country_code=*%p)\n", uid, country_code);

    auto* user = User::getUser(uid);
    if (!user)
        return SCE_NP_ERROR_USER_NOT_FOUND;

    if (!user->is_logged_in_psn)
        return SCE_NP_ERROR_SIGNED_OUT;

    // Stubbed
    std::strncpy(country_code->data, "us", 2);
    country_code->term = '\0';
    country_code->padding[0] = 0;
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

s32 PS4_FUNC sceNpGetParentalControlInfoA(s32 req_id, SceUserService::SceUserServiceUserId uid, s8* age, SceNpParentalControlInfo* info) {
    log("sceNpGetParentalControlInfoA(req_id=%d, uid=%d, age=*%p, info=*%p)\n", req_id, uid, age, info);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    *age = 18;
    info->content_restriction = false;
    info->chat_restriction = false;
    info->ugc_restriction = false;

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = SCE_OK;
    }
    return SCE_OK;
}

s32 PS4_FUNC sceNpCheckNpAvailabilityA(s32 req_id, SceUserService::SceUserServiceUserId uid) {
    log("sceNpCheckAvailabilityA(req_id=%d, uid=%d)\n", req_id, uid);

    auto* req = OS::find<SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    auto ret = SCE_OK;
    
    auto* user = User::getUser(uid);
    if (!user) {
        ret = SCE_NP_ERROR_USER_NOT_FOUND;
    }
    else if (!user->is_logged_in_psn) {
        ret = SCE_NP_ERROR_SIGNED_OUT;
    }

    if (req->is_async) {
        req->state = SceNpRequest::State::Finished;
        req->result = ret;
    }
    return ret;
}

}   // End namespace PS4::OS::Libs::SceNpManager