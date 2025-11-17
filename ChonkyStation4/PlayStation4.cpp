#include "PlayStation4.hpp"
#include <Loaders/Linker/Linker.hpp>
#include <Loaders/App.hpp>
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
        PS4::OS::Thread::init();
        g_app = PS4::Loader::Linker::loadAndLink(path);
        g_app.name = path.filename().generic_string();
        g_app.run();
    }
    catch (std::runtime_error e) {
        printf(e.what());
        return;
    }
}

}   // End namespace PS4