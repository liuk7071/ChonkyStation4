#include <Common.hpp>
#include <Loaders/Linker/Linker.hpp>
#include <Loaders/App.hpp>


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
        App app = PS4::Loader::Linker::loadAndLink(file);
        app.run();
    }
    catch (std::runtime_error e) {
        printf(e.what());
        return -1;
    }

    return 0;
}