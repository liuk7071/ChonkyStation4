#include <Common.hpp>
#include <PlayStation4.hpp>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/mman.h>
#endif


int main(int argc, char** argv) {
    printf("ChonkyStation4\n\n");

    // Try to reserve some address space as early as possible
#ifdef _WIN32
    void* ret = VirtualAlloc((void*)0x0'8000'0000, 2048_GB, MEM_RESERVE, PAGE_NOACCESS);
    if (!ret) {
        printf("Warning: failed to reserve address space\n");
    }
#else
    void* ret = mmap((void*)0x0'8000'0000, 2048_GB, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (ret == MAP_FAILED) {
        printf("Warning: failed to reserve address space\n");
    }
#endif

    fs::path file = "";
    if (argc >= 2)
        file = argv[1];
    else {
        printf("Usage: ChonkyStation4 [path to elf or game]\n");
        return -1;
    }

    PS4::loadAndRun(file);
    return 0;
}