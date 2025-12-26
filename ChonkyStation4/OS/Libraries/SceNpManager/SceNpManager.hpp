#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>


class Module;

namespace PS4::OS::Libs::SceNpManager {

void init(Module& module);

enum class SceNpState {
    SCE_NP_STATE_UNKNOWN = 0,
    SCE_NP_STATE_SIGNED_OUT,
    SCE_NP_STATE_SIGNED_IN
};

s32 PS4_FUNC sceNpGetState(SceUserService::SceUserServiceUserId uid, SceNpState* state);

}   // End namespace PS4::OS::Libs::SceNpManager