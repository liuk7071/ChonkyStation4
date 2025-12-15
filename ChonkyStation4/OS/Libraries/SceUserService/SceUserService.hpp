#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceUserService {

void init(Module& module);

using SceUserServiceUserId = s32;

enum class SceUserServiceEventType {
    Login = 0,
    Logout = 1
};

struct SceUserServiceEvent {
    SceUserServiceEventType event;
    SceUserServiceUserId user_id;
};

static constexpr s32 SCE_USER_SERVICE_MAX_LOGIN_USERS = 4;
static constexpr s32 SCE_USER_SERVICE_USER_ID_INVALID = -1;

static constexpr s32 SCE_USER_SERVICE_ERROR_NO_EVENT            = 0x80960007;
static constexpr s32 SCE_USER_SERVICE_ERROR_BUFFER_TOO_SHORT    = 0x8096000A;

static constexpr s32 SCE_DEVICE_SERVICE_ERROR_INVALID_USER = 0x809B0001;

struct SceUserServiceLoginUserIdList {
    SceUserServiceUserId user_ids[SCE_USER_SERVICE_MAX_LOGIN_USERS];
};

s32 PS4_FUNC sceUserServiceGetEvent(SceUserServiceEvent* event);
s32 PS4_FUNC sceUserServiceGetInitialUser(SceUserServiceUserId* user_id);
s32 PS4_FUNC sceUserServiceGetLoginUserIdList(SceUserServiceLoginUserIdList* user_id_list);
s32 PS4_FUNC sceUserServiceGetUserName(const SceUserServiceUserId user_id, char* username, const size_t size);

}   // End namespace PS4::OS::Libs::SceUserService