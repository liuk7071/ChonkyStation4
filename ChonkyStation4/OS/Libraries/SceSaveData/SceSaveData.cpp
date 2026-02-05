#include "SceSaveData.hpp"
#include <Logger.hpp>
#include <Loaders/App.hpp>
#include <OS/UserManagement.hpp>
#include <OS/Filesystem.hpp>


extern App g_app;

namespace PS4::OS::Libs::SceSaveData {

MAKE_LOG_FUNCTION(log, lib_sceSaveData);

void init(Module& module) {
    module.addSymbolExport("32HQAQdwM2o", "sceSaveDataMount", "libSceSaveData", "libSceSaveData", (void*)&sceSaveDataMount);
    module.addSymbolExport("0z45PIH+SNI", "sceSaveDataMount2", "libSceSaveData", "libSceSaveData", (void*)&sceSaveDataMount2);
    module.addSymbolExport("v7AAAMo0Lz4", "sceSaveDataSetupSaveDataMemory", "libSceSaveData", "libSceSaveData", (void*)&sceSaveDataSetupSaveDataMemory);
    module.addSymbolExport("7Bt5pBC-Aco", "sceSaveDataGetSaveDataMemory", "libSceSaveData", "libSceSaveData", (void*)&sceSaveDataGetSaveDataMemory);
    module.addSymbolExport("h3YURzXGSVQ", "sceSaveDataSetSaveDataMemory", "libSceSaveData", "libSceSaveData", (void*)&sceSaveDataSetSaveDataMemory);
    module.addSymbolExport("dyIhnXq-0SM", "sceSaveDataDirNameSearch", "libSceSaveData", "libSceSaveData", (void*)sceSaveDataDirNameSearch);
    module.addSymbolExport("ANmSWUiyyGQ", "sceSaveDataGetProgress", "libSceSaveData", "libSceSaveData", (void*)sceSaveDataGetProgress);
    
    module.addSymbolStub("ZkZhskCPXFw", "sceSaveDataInitialize", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("TywrFKCoLGY", "sceSaveDataInitialize3", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("c88Yy54Mx0w", "sceSaveDataSaveIcon", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("85zul--eGXs", "sceSaveDataSetParam", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("XgvSuIdnMlw", "sceSaveDataGetParam", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("oQySEUfgXRA", "sceSaveDataSetupSaveDataMemory2", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("QwOO7vegnV8", "sceSaveDataGetSaveDataMemory2", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("BMR4F-Uek3E", "sceSaveDataUmount", "libSceSaveData", "libSceSaveData");
}

s32 PS4_FUNC sceSaveDataMount(const SceSaveDataMount* mount, SceSaveDataMountResult* mount_result) {
    log("sceSaveDataMount(mount=*%p, mount_result=*%p)\n", mount, mount_result);

    SceSaveDataMount2 mount2;
    mount2.user_id      = mount->user_id;
    mount2.dir_name     = mount->dir_name;
    mount2.blocks       = mount->blocks;
    mount2.mount_mode   = mount->mount_mode;
    return sceSaveDataMount2(&mount2, mount_result);
}

s32 PS4_FUNC sceSaveDataMount2(const SceSaveDataMount2* mount, SceSaveDataMountResult* mount_result) {
    log("sceSaveDataMount2(mount=*%p, mount_result=*%p)\n", mount, mount_result);
    log("uid=%d, dir_name=\"%s\"\n", mount->user_id, mount->dir_name->data);
    
    // Get the user's savedata directory
    const auto user_save_dir = User::getUser(mount->user_id)->getHomeDir() / "savedata";
    // Get the specified savedata directory
    const auto mountpoint = user_save_dir / g_app.title_id / std::string(mount->dir_name->data);
    
    // Ensure the directory exists and mount it
    // TODO: We only support 1 savedata mountpoint for now
    fs::create_directories(mountpoint);
    FS::mount(FS::Device::SAVEDATA0, mountpoint);

    // Copy to mount result
    std::memset(mount_result->mount_point.data, 0, SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE);
    std::strcpy(mount_result->mount_point.data, "/savedata0");
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataSetupSaveDataMemory(SceUserService::SceUserServiceUserId user_id, size_t memory_size, SceSaveDataParam* param) {
    log("sceSaveDataSetupSaveDataMemory(user_id=%d, memory_size=0x%llx, param=*%p)\n", user_id, memory_size, param);

    if (memory_size == 0)
        Helpers::panic("sceSaveDataSetupSaveDataMemory: memory_size is 0\n");

    // TODO: Param

    // Get the user's savedata directory
    const auto user_save_dir = User::getUser(user_id)->getHomeDir() / "savedata";
    // Get the savedata memory path
    const auto savedata_memory_path = user_save_dir / g_app.title_id / "sce_savedata.dat";
    // Ensure the directory exists
    fs::create_directories(savedata_memory_path.parent_path());

    // Create savedata file if it didn't exist
    if (!fs::exists(savedata_memory_path)) {
        auto new_file = std::ofstream(savedata_memory_path, std::ios::binary);
        const char zero = 0;
        new_file.seekp(memory_size - 1);
        new_file.write(&zero, 1);
    }
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataGetSaveDataMemory(const SceUserService::SceUserServiceUserId user_id, void* buf, const size_t buf_size, const s64 offset) {
    log("sceSaveDataGetSaveDataMemory(user_id=%d, buf=%p, buf_size=0x%llx, offset=0x%llx)\n", user_id, buf, buf_size, offset);

    // Get the user's savedata directory
    const auto user_save_dir = User::getUser(user_id)->getHomeDir() / "savedata";
    // Get the savedata memory path
    const auto savedata_memory_path = user_save_dir / g_app.title_id / "sce_savedata.dat";
    // Error if the file does not exist
    if (!fs::exists(savedata_memory_path))
        return SCE_SAVE_DATA_ERROR_MEMORY_NOT_READY;

    std::ifstream data = std::ifstream(savedata_memory_path, std::ios::binary);
    data.seekg(offset, std::ios::beg);
    data.read((char*)buf, buf_size);
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataSetSaveDataMemory(const SceUserService::SceUserServiceUserId user_id, void* buf, const size_t buf_size, const s64 offset) {
    log("sceSaveDataSetSaveDataMemory(user_id=%d, buf=%p, buf_size=0x%llx, offset=0x%llx)\n", user_id, buf, buf_size, offset);

    // Get the user's savedata directory
    const auto user_save_dir = User::getUser(user_id)->getHomeDir() / "savedata";
    // Get the savedata memory path
    const auto savedata_memory_path = user_save_dir / g_app.title_id / "sce_savedata.dat";
    // Error if the file does not exist
    if (!fs::exists(savedata_memory_path))
        return SCE_SAVE_DATA_ERROR_MEMORY_NOT_READY;

    std::ofstream data = std::ofstream(savedata_memory_path, std::ios::binary);
    data.seekp(offset, std::ios::beg);
    data.write((char*)buf, buf_size);
    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataDirNameSearch(const SceSaveDataDirNameSearchCond* cond, SceSaveDataDirNameSearchResult* result) {
    log("sceSaveDataDirNameSearch(cond=*%p, result=*%p) TODO\n", cond, result);

    const auto dir_name = cond->dir_name ? std::string(cond->dir_name->data) : "";
    log("dir_name=\"%s\"\n", dir_name.c_str());

    // If a dir_name is specified, check if this directory exists.
    // TODO: Wildcards
    if (!dir_name.empty()) {
        // Get the user's savedata directory
        const auto user_save_dir = User::getUser(cond->user_id)->getHomeDir() / "savedata";
        // Get the game's savedata directory
        const auto game_dir = user_save_dir / g_app.title_id;

        // Check if the specified directory exists
        if (fs::exists(game_dir / dir_name)) {
            result->n_hits = 1;
            if (result->n_dir_names > 0) {
                result->n_dir_names_set = 1;
                std::strcpy(result->dir_names[0].data, dir_name.c_str());
            }
            if (result->infos) {
                // TODO
                result->infos[0].blocks = 100;
                result->infos[0].free_blocks = 1000;
            }
        }
        else {
            result->n_hits = 0;
            result->n_dir_names_set = 0;
        }
    }
    else Helpers::panic("TODO: sceSaveDataDirNameSearch with empty dir_name");

    return SCE_OK;
}

s32 PS4_FUNC sceSaveDataGetProgress(float* progress) {
    log("sceSaveDataGetProgress(progress=*%p)\n", progress);

    *progress = 1.0f;   // Done
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceSysmodule