#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceUserService {

void init(Module& module);

using SceUserServiceUserId = s32;

static constexpr s32 SCE_USER_SERVICE_MAX_LOGIN_USERS = 4;
static constexpr s32 SCE_USER_SERVICE_USER_ID_INVALID = -1;

static constexpr s32 SCE_USER_SERVICE_ERROR_BUFFER_TOO_SHORT = 0x8096000A;

struct SceUserServiceLoginUserIdList {
    SceUserServiceUserId user_ids[SCE_USER_SERVICE_MAX_LOGIN_USERS];
};

s32 PS4_FUNC sceUserServiceGetInitialUser(SceUserServiceUserId* user_id);
s32 PS4_FUNC sceUserServiceGetLoginUserIdList(SceUserServiceLoginUserIdList* user_id_list);
s32 PS4_FUNC sceUserServiceGetUserName(const SceUserServiceUserId user_id, char* username, const size_t size);

}   // End namespace PS4::OS::Libs::SceUserService