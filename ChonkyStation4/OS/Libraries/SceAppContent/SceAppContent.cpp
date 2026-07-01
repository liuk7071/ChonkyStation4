#include "SceAppContent.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceAppContent {

MAKE_LOG_FUNCTION(log, lib_sceAppContent);

void init(Module& module) {
    module.addSymbolExport("7bOLX66Iz-U", "sceAppContentTemporaryDataMount", "libSceAppContent", "libSceAppContentUtil", (void*)&sceAppContentTemporaryDataMount);
    module.addSymbolExport("buYbeLOGWmA", "sceAppContentTemporaryDataMount2", "libSceAppContent", "libSceAppContentUtil", (void*)&sceAppContentTemporaryDataMount2);
    module.addSymbolExport("SaKib2Ug0yI", "sceAppContentTemporaryDataGetAvailableSpaceKb", "libSceAppContent", "libSceAppContentUtil", (void*)&sceAppContentTemporaryDataGetAvailableSpaceKb);
    module.addSymbolExport("xnd8BJzAxmk", "sceAppContentGetAddcontInfoList", "libSceAppContent", "libSceAppContentUtil", (void*)&sceAppContentGetAddcontInfoList);
    
    module.addSymbolStub("R9lA82OraNs", "sceAppContentInitialize", "libSceAppContent", "libSceAppContentUtil");
    module.addSymbolStub("99b82IKXpH4", "sceAppContentAppParamGetInt", "libSceAppContent", "libSceAppContentUtil"); // TODO: Important
    //module.addSymbolStub("XTWR0UXvcgs", "sceAppContentGetEntitlementKey", "libSceAppContent", "libSceAppContentUtil", 0x80D90007 /* SCE_APP_CONTENT_ERROR_DRM_NO_ENTITLEMENT */); // TODO: Important...?
    module.addSymbolStub("XTWR0UXvcgs", "sceAppContentGetEntitlementKey", "libSceAppContent", "libSceAppContentUtil", 0); // TODO: Important...?
    module.addSymbolStub("a5N7lAG0y2Q", "sceAppContentTemporaryDataFormat", "libSceAppContent", "libSceAppContentUtil");
    //module.addSymbolStub("VANhIWcqYak", "sceAppContentAddcontMount", "libSceAppContent", "libSceAppContentUtil", 0x80D90005 /* SCE_APP_CONTENT_ERROR_NOT_FOUND */);
}

// TODO: I'm unsure if the function signature is the same as the 2 variant, but I think it is
s32 PS4_FUNC sceAppContentTemporaryDataMount(SceAppContentTemporaryDataOption option, SceAppContentMountPoint* mount_point) {
    log("sceAppContentTemporaryDataMount(option=%d, mount_point=*%p)\n", option, mount_point);
    return sceAppContentTemporaryDataMount2(option, mount_point);
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

s32 PS4_FUNC sceAppContentGetAddcontInfoList(Np::SceNpServiceLabel service_label, SceAppContentAddcontInfo* list, u32 n_list, u32* n_hit) {
    log("sceAppContentGetAddcontInfoList(service_label=%x, list=*%p, n_list=%d, n_hit=*%p)\n", service_label, list, n_list, n_hit);

    // TODO
    *n_hit = 0;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceAppContent