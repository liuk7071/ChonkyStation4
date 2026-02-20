#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNpManager {

void init(Module& module);

using namespace OS::Np;

struct SceNpCreateAsyncRequestParameter;

struct SceNpRequest : SceObj {
    enum class State {
        Running,
        Finished
    };
    
    bool is_async   = false;
    State state     = State::Running;   // For async requests
    s32 result      = 0;                // For async requests
};

s32 PS4_FUNC sceNpCheckCallback();
s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state);
s32 PS4_FUNC sceNpGetNpId(SceUserService::SceUserServiceUserId uid, SceNpId* np_id);
s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id);
s32 PS4_FUNC sceNpCreateRequest();
s32 PS4_FUNC sceNpCreateAsyncRequest(const SceNpCreateAsyncRequestParameter* param);
s32 PS4_FUNC sceNpPollAsync(s32 req_id, s32* result);
s32 PS4_FUNC sceNpCheckPlus(s32 req_id, const SceNpCheckPlusParameter* param, SceNpCheckPlusResult* result);
s32 PS4_FUNC sceNpGetParentalControlInfo(s32 req_id, SceNpOnlineId* online_id, s8* age, SceNpParentalControlInfo* info);

}   // End namespace PS4::OS::Libs::SceNpManager