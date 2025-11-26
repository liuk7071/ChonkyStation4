#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceUserService {

void init(Module& module);

using SceUserServiceUserId = s32;

static constexpr s32 SCE_USER_SERVICE_MAX_LOGIN_USERS = 4;
static constexpr s32 SCE_USER_SERVICE_USER_ID_INVALID = -1;

struct SceUserServiceLoginUserIdList {
    SceUserServiceUserId user_ids[SCE_USER_SERVICE_MAX_LOGIN_USERS];
};

s32 PS4_FUNC sceUserServiceGetInitialUser(SceUserServiceUserId* user_id);
s32 PS4_FUNC sceUserServiceGetLoginUserIdList(SceUserServiceLoginUserIdList* user_id_list);

}   // End namespace PS4::OS::Libs::SceUserService