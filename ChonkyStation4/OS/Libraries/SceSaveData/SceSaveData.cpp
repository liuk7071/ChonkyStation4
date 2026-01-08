#include "SceSaveData.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSaveData {

MAKE_LOG_FUNCTION(log, lib_sceSaveData);

void init(Module& module) {
    module.addSymbolExport("32HQAQdwM2o", "sceSaveDataMount", "libSceSaveData", "libSceSaveData", (void*)&sceSaveDataMount);
    
    module.addSymbolStub("ZkZhskCPXFw", "sceSaveDataInitialize", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("TywrFKCoLGY", "sceSaveDataInitialize3", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("0z45PIH+SNI", "sceSaveDataMount2", "libSceSaveData", "libSceSaveData", 0x809f000a /* no space */);
    module.addSymbolStub("v7AAAMo0Lz4", "sceSaveDataSetupSaveDataMemory", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("7Bt5pBC-Aco", "sceSaveDataGetSaveDataMemory", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("h3YURzXGSVQ", "sceSaveDataSetSaveDataMemory", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("dyIhnXq-0SM", "sceSaveDataDirNameSearch", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("c88Yy54Mx0w", "sceSaveDataSaveIcon", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("BMR4F-Uek3E", "sceSaveDataUmount", "libSceSaveData", "libSceSaveData");
}

s32 PS4_FUNC sceSaveDataMount(const SceSaveDataMount2* mount, SceSaveDataMountResult* mount_result) {
    log("sceSaveDataMount(mount=*%p, mount_result=*%p) TODO\n");

    std::memset(mount_result->mount_point.data, 0, SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE);
    std::strcpy(mount_result->mount_point.data, "/savedata0");
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataDirNameSearch(const SceSaveDataDirNameSearchCond* cond, SceSaveDataDirNameSearchResult* result) {
    log("sceSaveDataDirNameSearch(cond=*%p, result=*%p) TODO\n");

    // TODO
    result->n_hits = 0;
    result->n_dir_names_set = 0;
    //result->n_hits = 2;
    //result->n_dir_names_set = 2;
    //std::strcpy(result->dir_names[0].data, "profile");
    //std::strcpy(result->dir_names[1].data, "savegame00");
    //if (result->infos) {
    //    result->infos[0].blocks = 100;
    //    result->infos[0].free_blocks = 1000;
    //    result->infos[1].blocks = 100;
    //    result->infos[1].free_blocks = 1000;
    //}
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceSysmodule