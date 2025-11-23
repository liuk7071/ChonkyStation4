#include "PlayStation4.hpp"
#include <Loaders/App.hpp>
#include <Loaders/App/AppLoader.hpp>
#include <Loaders/Linker/Linker.hpp>
#include <OS/Thread.hpp>
#include <GCN/GCN.hpp>


App g_app;

namespace PS4 {

void init() {
    PS4::GCN::initVulkan();
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

        g_app.run();
    }
    catch (std::runtime_error e) {
        printf(e.what());
        return;
    }
}

}   // End namespace PS4