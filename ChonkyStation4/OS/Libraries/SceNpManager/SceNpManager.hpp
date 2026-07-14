#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNpManager {

void init(Module& module);

using namespace OS::Np;

static constexpr s32 SCE_NP_LANGUAGE_CODE_MAX_LEN = 5;

struct SceNpCreateAsyncRequestParameter;

struct SceNpLanguageCode {
    char code[SCE_NP_LANGUAGE_CODE_MAX_LEN + 1];
    u8 padding[10];
};

struct SceNpRequest : SceObj {
    enum class State {
        Running,
        Finished
    };
    
    bool is_async   = false;
    State state     = State::Running;   // For async requests
    s32 result      = 0;                // For async requests

    void finish(s32 res) {
        state = State::Finished;
        result = res;
    }
};

s32 PS4_FUNC sceNpCheckCallback();
s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state);
s32 PS4_FUNC sceNpGetNpId(SceUserService::SceUserServiceUserId uid, SceNpId* np_id);
s32 PS4_FUNC sceNpHasSignedUp(SceUserService::SceUserServiceUserId uid, bool* has_signed_up);
s32 PS4_FUNC sceNpGetAccountIdA(SceUserService::SceUserServiceUserId uid, SceNpAccountId* account_id);
s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id);
s32 PS4_FUNC sceNpGetUserIdByAccountId(SceNpAccountId account_id, SceUserService::SceUserServiceUserId* uid);
s32 PS4_FUNC sceNpGetAccountCountryA(SceUserService::SceUserServiceUserId uid, SceNpCountryCode* country_code);
s32 PS4_FUNC sceNpCreateRequest();
s32 PS4_FUNC sceNpCreateAsyncRequest(const SceNpCreateAsyncRequestParameter* param);
s32 PS4_FUNC sceNpPollAsync(s32 req_id, s32* result);
s32 PS4_FUNC sceNpCheckPlus(s32 req_id, const SceNpCheckPlusParameter* param, SceNpCheckPlusResult* result);
s32 PS4_FUNC sceNpGetParentalControlInfo(s32 req_id, SceNpOnlineId* online_id, s8* age, SceNpParentalControlInfo* info);
s32 PS4_FUNC sceNpGetParentalControlInfoA(s32 req_id, SceUserService::SceUserServiceUserId uid, s8* age, SceNpParentalControlInfo* info);
s32 PS4_FUNC sceNpGetAccountLanguageA(s32 req_id, SceUserService::SceUserServiceUserId uid, s8* age, SceNpLanguageCode* lang_code);
s32 PS4_FUNC sceNpCheckNpAvailabilityA(s32 req_id, SceUserService::SceUserServiceUserId uid);

}   // End namespace PS4::OS::Libs::SceNpManager