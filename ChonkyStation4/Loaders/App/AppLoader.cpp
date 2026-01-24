#include "AppLoader.hpp"
#include <Loaders/SFO/SFOLoader.hpp>
#include <Loaders/Linker/Linker.hpp>
#include <OS/Filesystem.hpp>
#include <SDL.h>    // For SDSL_GetPrefPath


namespace PS4::Loader::App {

bool prepareApp(const fs::path& app_content_path, AppInfo& info) {
    info.content_path       = app_content_path;
    info.param_sfo_path     = info.content_path / "sce_sys/param.sfo";
    info.executable_path    = info.content_path / "eboot.bin";
    info.icon0_path         = info.content_path / "sce_sys/icon0.png";
    info.pic1_path          = info.content_path / "sce_sys/pic1.png";

    // Verify that at least param.sfo and eboot.bin exist
    if (!fs::exists(info.param_sfo_path) || !fs::exists(info.executable_path)) {
        return false;
    }

    // Parse param.sfo
    auto sfo = Loader::SFO::parse(info.param_sfo_path);
    // Verify at least these fields exist to ensure SFO is valid
    if (!sfo.strings.contains("TITLE") || !sfo.strings.contains("APP_VER") || !sfo.strings.contains("CONTENT_ID")) {
        return false;
    }

    info.title    = std::string(reinterpret_cast<const char*>(sfo.strings["TITLE"].c_str()));
    info.title_id = std::string(reinterpret_cast<const char*>(sfo.strings["TITLE_ID"].c_str()));
    info.version  = std::string(reinterpret_cast<const char*>(sfo.strings["APP_VER"].c_str()));
    return true;
}

bool getApp(const AppInfo& info, ::App& app) {
    fs::path tmp_path = info.executable_path;
    app = std::move(Loader::Linker::loadAndLink(tmp_path));

    // TODO: Just have an AppInfo inside App?
    app.name = info.title;
    app.title_id = info.title_id;
    
    const std::string sysmodules_to_load[] = {
        "libSceLibcInternal.sprx",
        "libSceNgs2.sprx",
        "libSceUlt.sprx",
        "libScePngDec.sprx"
    };

    const std::string partial_lle_sysmodules_to_load[] = {
        "libSceGnmDriver.sprx"
    };

    const fs::path sysmodules_path = fs::path(SDL_GetPrefPath("ChonkyStation", "ChonkyStation4")) / "sysmodules";
    fs::create_directories(sysmodules_path);    // Ensure directory exists

    // Load system modules. For now it's a hardcoded path
    for (auto& sysmodule : sysmodules_to_load) {
        const auto sysmodule_path = sysmodules_path / sysmodule;
        if (!fs::exists(sysmodule_path)) {
            Helpers::panic("Required sysmodule %s does not exist\n", sysmodule.c_str());
        }

        Loader::Linker::loadAndLinkLib(app, sysmodule_path);
    }

    for (auto& sysmodule : partial_lle_sysmodules_to_load) {
        const auto sysmodule_path = sysmodules_path / sysmodule;
        if (!fs::exists(sysmodule_path)) {
            Helpers::panic("Required sysmodule %s does not exist\n", sysmodule.c_str());
        }

        Loader::Linker::loadAndLinkLib(app, sysmodule_path, true, app.getHLEModule());
    }

    // Load game modules from the "sce_module" folder
    for (auto& module : fs::directory_iterator(info.content_path / "sce_module")) {
        // Paths with weird characters cause exceptions
        try {
            module.path().generic_string();
        }
        catch (...) {
            continue;
        }

        auto ext = module.path().extension();
        if (ext != ".prx" && ext != ".sprx") continue;

        // For now we expect decrypted modules as .elf files
        auto module_path = module.path();
        if (!fs::exists(module_path)) continue;

        Loader::Linker::loadAndLinkLib(app, module_path);
    }

    // Mount /app0 and initialize FS
    FS::mount(FS::Device::APP0, info.content_path);

    return true;
}

}   // End namespace PS4::Loader::App