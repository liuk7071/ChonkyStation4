#include <Common.hpp>
#include <Loaders/Linker/Linker.hpp>
#include <Loaders/App.hpp>
#include <OS/Thread.hpp>


App g_app;

int main(int argc, char** argv) {
    printf("ChonkyStation4\n\n");

    fs::path file = "";
    if (argc >= 2)
        file = argv[1];
    else {
        printf("Usage: ChonkyStation4 [path to elf]\n");
        return -1;
    }

    try {
        PS4::OS::Thread::init();
        g_app = PS4::Loader::Linker::loadAndLink(file);
        g_app.run();
    }
    catch (std::runtime_error e) {
        printf(e.what());
        return -1;
    }

    return 0;
}