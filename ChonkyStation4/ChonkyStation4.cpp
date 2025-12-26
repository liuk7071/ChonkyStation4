#include <Common.hpp>
#include <PlayStation4.hpp>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif


int main(int argc, char** argv) {
    printf("ChonkyStation4\n\n");

    // Try to reserve some address space as early as possible
#ifdef _WIN32
    void* ret = VirtualAlloc((void*)0x2'0000'0000, 1024_GB, MEM_RESERVE, PAGE_NOACCESS);
    if (!ret) {
        printf("Warning: failed to reserve address space\n");
    }
#endif

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