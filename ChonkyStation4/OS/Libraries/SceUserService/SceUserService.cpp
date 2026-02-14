#include "SceUserService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/UserManagement.hpp>


namespace PS4::OS::Libs::SceUserService {

MAKE_LOG_FUNCTION(log, lib_sceUserService);

void init(Module& module) {
    module.addSymbolExport("yH17Q6NWtVg", "sceUserServiceGetEvent", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetEvent);
    module.addSymbolExport("CdWp0oHWGr0", "sceUserServiceGetInitialUser", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetInitialUser);
    module.addSymbolExport("fPhymKNvK-A", "sceUserServiceGetLoginUserIdList", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetLoginUserIdList);
    module.addSymbolExport("1xxcMiGu2fo", "sceUserServiceGetUserName", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetUserName);
    
    module.addSymbolStub("j3YMu1MVNNo", "sceUserServiceInitialize", "libSceUserService", "libSceUserService");
    module.addSymbolStub("az-0R6eviZ0", "sceUserServiceInitialize2", "libSceUserService", "libSceUserService");
    module.addSymbolStub("lUoqwTQu4Go", "sceUserServiceGetUserColor", "libSceUserService", "libSceUserService");

    module.addSymbolStub("wuI7c7UNk0A", "sceUserServiceRegisterEventCallback", "libSceUserServiceForNpToolkit", "libSceUserService");
}

bool is_logged_in = false;

s32 PS4_FUNC sceUserServiceGetEvent(SceUserServiceEvent* event) {
    log("sceUserServiceGetEvent(event=*%p)\n", event);
    
    // Send a login event
    if (!is_logged_in) {
        is_logged_in = true;
        event->event = SceUserServiceEventType::Login;
        event->user_id = User::current->getID();
        return SCE_OK;
    }

    return SCE_USER_SERVICE_ERROR_NO_EVENT;
}

s32 PS4_FUNC sceUserServiceGetInitialUser(SceUserServiceUserId* user_id) {
    log("sceUserServiceGetInitialUser(user_id=*%p)\n", user_id);

    *user_id = User::current->getID();
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetLoginUserIdList(SceUserServiceLoginUserIdList* user_id_list) {
    log("sceUserServiceGetLoginUserIdList(user_id_list=*%p)\n", user_id_list);

    for (int i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; i++)
        user_id_list->user_ids[i] = SCE_USER_SERVICE_USER_ID_INVALID;

    // Return 1 logged in user
    user_id_list->user_ids[0] = User::current->getID();
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetUserName(const SceUserServiceUserId user_id, char* username, const size_t size) {
    log("sceUserServiceGetUserName(user_id=%d, username=*%p, size=%lld)\n", user_id, username, size);

    const std::string name = User::current->getUsername();
    if (size < name.size()) return SCE_USER_SERVICE_ERROR_BUFFER_TOO_SHORT;

    std::memset(username, '\0', size);
    std::strcpy(username, name.c_str());
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager