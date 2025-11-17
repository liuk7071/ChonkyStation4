#include <Common.hpp>
#include <PlayStation4.hpp>


int main(int argc, char** argv) {
    printf("ChonkyStation4\n\n");

    fs::path file = "";
    if (argc >= 2)
        file = argv[1];
    else {
        printf("Usage: ChonkyStation4 [path to elf]\n");
        return -1;
    }

    PS4::loadAndRun(file);
    return 0;
}