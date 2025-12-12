#include "SceUserService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceUserService {

MAKE_LOG_FUNCTION(log, lib_sceUserService);

void init(Module& module) {
    module.addSymbolExport("CdWp0oHWGr0", "sceUserServiceGetInitialUser", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetInitialUser);
    module.addSymbolExport("fPhymKNvK-A", "sceUserServiceGetLoginUserIdList", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetLoginUserIdList);
    module.addSymbolExport("1xxcMiGu2fo", "sceUserServiceGetUserName", "libSceUserService", "libSceUserService", (void*)&sceUserServiceGetUserName);
    
    module.addSymbolStub("j3YMu1MVNNo", "sceUserServiceInitialize", "libSceUserService", "libSceUserService");
    module.addSymbolStub("yH17Q6NWtVg", "sceUserServiceGetEvent", "libSceUserService", "libSceUserService", 0x80960007 /* SCE_USER_SERVICE_ERROR_NO_EVENT */);
}

s32 PS4_FUNC sceUserServiceGetInitialUser(SceUserServiceUserId* user_id) {
    log("sceUserServiceGetInitialUser(user_id=*%p)\n", user_id);

    *user_id = 100;
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetLoginUserIdList(SceUserServiceLoginUserIdList* user_id_list) {
    log("sceUserServiceGetLoginUserIdList(user_id_list=*%p)\n", user_id_list);

    for (int i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; i++)
        user_id_list->user_ids[i] = SCE_USER_SERVICE_USER_ID_INVALID;

    // Return 1 logged in user
    user_id_list->user_ids[0] = 100;
    return SCE_OK;
}

s32 PS4_FUNC sceUserServiceGetUserName(const SceUserServiceUserId user_id, char* username, const size_t size) {
    log("sceUserServiceGetUserName(user_id=%d, username=*%p, size=%lld)\n", user_id, username, size);

    const std::string name = "Alber";
    if (size < name.size()) return SCE_USER_SERVICE_ERROR_BUFFER_TOO_SHORT;

    std::strcpy(username, name.c_str());
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager