#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNpManager {

void init(Module& module);

using namespace OS::Np;

static constexpr s32 SCE_NP_ERROR_CALLBACK_ALREADY_REGISTERED = 0x80550008;

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

enum SceNpGamePresenceStatus {
    SCE_NP_GAME_PRESENCE_STATUS_OFFLINE,
    SCE_NP_GAME_PRESENCE_STATUS_ONLINE
};

using SceNpStateCallback        = PS4_FUNC void (*)(SceUserService::SceUserServiceUserId uid, SceNpState state, SceNpId* np_id, void* userdata);
using SceNpGamePresenceCallback = PS4_FUNC void (*)(const SceNpOnlineId* online_id, SceNpGamePresenceStatus status, void* userdata);

void pushStateEvent(const SceUserService::SceUserServiceUserId& uid, const SceNpState& state);
void pushPresenceEvent(const SceUserService::SceUserServiceUserId& uid, const SceNpGamePresenceStatus& status);

s32 PS4_FUNC sceNpCheckCallback();
s32 PS4_FUNC sceNpRegisterStateCallback(SceNpStateCallback callback, void* userdata);
s32 PS4_FUNC sceNpRegisterGamePresenceCallback(SceNpGamePresenceCallback callback, void* userdata);
s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state);
s32 PS4_FUNC sceNpGetGamePresenceStatus(SceNpOnlineId* online_id, SceNpGamePresenceStatus* status);
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
s32 PS4_FUNC sceNpCheckNpAvailability(s32 req_id, const Np::SceNpOnlineId* online_id, void* reserved);
s32 PS4_FUNC sceNpCheckNpAvailabilityA(s32 req_id, SceUserService::SceUserServiceUserId uid);

}   // End namespace PS4::OS::Libs::SceNpManager