#include "SceAppContent.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceAppContent {

MAKE_LOG_FUNCTION(log, lib_sceAppContent);

void init(Module& module) {
    module.addSymbolExport("buYbeLOGWmA", "sceAppContentTemporaryDataMount2", "libSceAppContent", "libSceAppContentUtil", (void*)&sceAppContentTemporaryDataMount2);
    module.addSymbolExport("SaKib2Ug0yI", "sceAppContentTemporaryDataGetAvailableSpaceKb", "libSceAppContent", "libSceAppContentUtil", (void*)&sceAppContentTemporaryDataGetAvailableSpaceKb);
    
    module.addSymbolStub("R9lA82OraNs", "sceAppContentInitialize", "libSceAppContent", "libSceAppContentUtil");
    module.addSymbolStub("xnd8BJzAxmk", "sceAppContentGetAddcontInfoList", "libSceAppContent", "libSceAppContentUtil");
    module.addSymbolStub("99b82IKXpH4", "sceAppContentAppParamGetInt", "libSceAppContent", "libSceAppContentUtil"); // TODO: Important
    //module.addSymbolStub("XTWR0UXvcgs", "sceAppContentGetEntitlementKey", "libSceAppContent", "libSceAppContentUtil", 0x80D90007 /* SCE_APP_CONTENT_ERROR_DRM_NO_ENTITLEMENT */); // TODO: Important...?
    module.addSymbolStub("XTWR0UXvcgs", "sceAppContentGetEntitlementKey", "libSceAppContent", "libSceAppContentUtil", 0); // TODO: Important...?
}

s32 PS4_FUNC sceAppContentTemporaryDataMount2(SceAppContentTemporaryDataOption option, SceAppContentMountPoint* mount_point) {
    log("sceAppContentTemporaryDataMount2(option=%d, mount_point=*%p)\n", option, mount_point);

    // TODO: Format option
    std::memset(mount_point->data, '\0', SCE_APP_CONTENT_MOUNTPOINT_DATA_MAXSIZE);
    std::strncpy(mount_point->data, "/temp0", SCE_APP_CONTENT_MOUNTPOINT_DATA_MAXSIZE);
    return SCE_OK;
}

s32 PS4_FUNC sceAppContentTemporaryDataGetAvailableSpaceKb(SceAppContentMountPoint* mount_point, size_t* available_space_kb) {
    log("sceAppContentTemporaryDataGetAvailableSpaceKb(mount_point=*%p, available_space_kb=*%p)\n", mount_point, available_space_kb);

    *available_space_kb = 1024 * 1024;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceAppContent