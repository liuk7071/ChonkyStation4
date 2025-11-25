#include "App.hpp"
#include <PlayStation4.hpp>
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

void initAndJumpToEntry(std::vector<Module>* modules) {
    PS4::init();

    printf("Initializing modules:\n", modules->size() - 1);
    for (int i = 0; i < modules->size(); i++) {
        if (i == 0) continue;

        auto& mod = (*modules)[i];
        if (mod.init_func) {
            if (mod.exported_modules.size())
                printf("- %s\n", mod.exported_modules[0].name.c_str());   // Use the name of the first exported module just to print something
            else printf("- unnamed module\n");   // Probably won't ever happen?
        }
    }

    // Initialize modules
    for (int i = 0; i < modules->size(); i++) {
        // Skip the first module, because the init func is usually just the entry point
        if (i == 0) continue;

        auto& mod = (*modules)[i];
        if (mod.init_func) {
            mod.init_func(0, nullptr, nullptr);
            if (mod.exported_modules.size())
                printf("Initialized module %s\n", mod.exported_modules[0].name.c_str());   // Use the name of the first exported module just to print something
            else printf("Initialized unnamed module\n");   // Probably won't ever happen?
        }
    }

    // Dummy arguments
    Params params;
    params.argc = 0;
    params.argv[0] = nullptr;
    params.entry = (*modules)[0].entry;

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
    : "r"(params.entry), "r"((u64)params.argc), "r"(params.argv[0]), "r"(&params), "r"(exitFunc)
    : "rax", "rsi", "rdi"
    );
}

void App::run() {
    Helpers::debugAssert(modules.size(), "App::run: no modules loaded\n");

    // Run app
    log("Running app\n");
    // Create main thread
    auto main_thread = PS4::OS::Thread::createThread("main", initAndJumpToEntry, &modules);
    PS4::OS::Thread::joinThread(main_thread);
}

std::pair<u8*, size_t> App::getTLSImage(u32 modid) {
    Helpers::debugAssert(modules.size(), "App::getTLSImage: no modules loaded\n");
    // Find module that contains the TLS block with id == modid
    for (auto& mod : modules) {
        if (mod.tls_modid == modid) {
            return { (u8*)mod.tls_vaddr, (size_t)mod.tls_size };
        }
    }

    Helpers::panic("Could not find TLS image with id %d\n", modid);    
}