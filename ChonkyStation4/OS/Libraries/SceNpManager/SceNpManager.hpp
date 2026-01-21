#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>


class Module;

namespace PS4::OS::Libs::SceNpManager {

void init(Module& module);

static constexpr s32 SCE_NP_ERROR_SIGNED_OUT = 0x80550006;

static constexpr s32 SCE_NP_ONLINEID_MAX_LENGTH = 16;

enum class SceNpState {
    SCE_NP_STATE_UNKNOWN = 0,
    SCE_NP_STATE_SIGNED_OUT,
    SCE_NP_STATE_SIGNED_IN
};

struct SceNpOnlineId {
    char data[SCE_NP_ONLINEID_MAX_LENGTH];
    char term;
    char dummy[3];
};

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state);
s32 PS4_FUNC sceNpGetOnlineId(SceUserService::SceUserServiceUserId uid, SceNpOnlineId* online_id);

}   // End namespace PS4::OS::Libs::SceNpManager