#include "App.hpp"
#include <OS/Thread.hpp>


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
    Helpers::debugAssert(modules.size(), "App::run: no modules loaded\n");

    // Create main thread
    auto main_thread = PS4::OS::Thread::createThread("main", jumpToEntry, modules[0].entry);
    PS4::OS::Thread::joinThread(main_thread);
}

std::pair<u8*, size_t> App::getTLSImage() {
    // For now return TLS image of the first module.
    // Later we need to combine TLS images of all modules and handle TLS relocations.
    Helpers::debugAssert(modules.size(), "App::getTLSImage: no modules loaded\n");
    return { (u8*)modules[0].tls_vaddr, (size_t)modules[0].tls_size };
}