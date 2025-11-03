#include "App.hpp"


void PS4_FUNC exitFunc() {
    printf("Called exit\n");
    exit(0);
}

struct Params {
    int argc;
    u32 padding;
    const char* argv[33];
    void* entry;
};

void jumpToEntry(void* entry) {
    // Dummy arguments
    Params params;
    params.argc = 0;
    params.argv[0] = nullptr;
    params.entry = entry;

    asm volatile(R"(
        # Align stack
        andq $-16, %%rsp
        subq $8, %%rsp
        
        # Push argc and argv
        pushq %2
        pushq %1
        
        movq %3, %%rdi  # Pointer to params struct
        movq %4, %%rsi  # Poiner to exit handler
        
        # Jump to the entry point
        jmp *%0
    )"
    :
    : "r"(entry), "r"((u64)params.argc), "r"(params.argv[0]), "r"(&params), "r"(exitFunc)
    :    
    );
}

void App::run() {
    log("Running app\n");
    jumpToEntry(modules[0].entry);
}