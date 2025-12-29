#include "PlayStation4.hpp"
#include <Loaders/App.hpp>
#include <Loaders/App/AppLoader.hpp>
#include <Loaders/Linker/Linker.hpp>
#include <OS/Thread.hpp>
#include <OS/Filesystem.hpp>
#include <GCN/GCN.hpp>
#include <thread>


App g_app;

namespace PS4 {

void init() {
    // Create GCN thread
    std::thread gcn_thread(GCN::gcnThread);
    gcn_thread.detach();

    FS::mount(FS::Device::SAVEDATA0, "./savedata0");
    FS::mount(FS::Device::DEV, "./dev");    // TODO: Properly handle /dev
    FS::init();

    // Wait for graphics initialization to complete
    while (!GCN::initialized) std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void loadAndRun(const fs::path& path) {
    try {
        // The threading system needs to be initialized before we run the app.
        // Everything else will be initialized in the init() function, which is called by g_app.run() from the app's main thread (NOT the host's)
        OS::Thread::init();

        // Use AppLoader if it's a game (aka if it's a directory), otherwise manually load and link if it's an elf
        if (fs::is_directory(path)) {
            Loader::App::AppInfo app_info;
            Loader::App::prepareApp(path, app_info);
            Loader::App::getApp(app_info, g_app);
        } else {
            g_app = std::move(Loader::Linker::loadAndLink(path));
            // TODO: Probably link sysmodules for ELFs too
            g_app.name = path.filename().generic_string();
        }

        // Log module base addresses (for debugging)
        std::ofstream log_out;
        log_out.open("log_baseaddress.txt");
        for (auto& mod : g_app.modules) {
            const auto base_address_str = std::format("{:016p}:\t{}\n", mod.base_address, mod.filename);
            log_out.write(base_address_str.c_str(), base_address_str.length());
        }
        log_out.close();

        g_app.run();
    }
    catch (std::runtime_error e) {
        printf(e.what());
        return;
    }
}

}   // End namespace PS4