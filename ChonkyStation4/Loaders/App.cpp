#include "App.hpp"
#include <PlayStation4.hpp>
#include <Configuration.hpp>
#include <NameToNid.hpp>
#include <OS/Thread.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>


void PS4_FUNC exitFunc() {
    printf("Called exit\n");
    exit(0);
}

void* PS4_FUNC initAndJumpToEntry(App* app) {
    PS4::init();

    auto* modules = &app->modules;

    printf("Initializing modules:\n", modules->size() - 1);
    for (int i = 0; i < modules->size(); i++) {
        if (i == 0) continue;

        auto& mod = (*modules)[i];
        if (mod->init_func) {
            if (mod->exported_modules.size())
                printf("- %s\n", mod->exported_modules[0].name.c_str());   // Use the name of the first exported module just to print something
            else printf("- unnamed module\n");   // Probably won't ever happen?
        }
    }

    // Initialize modules
    for (int i = 0; i < modules->size(); i++) {
        // Skip the first module, because the init func is usually just the entry point
        if (i == 0) continue;

        auto& mod = (*modules)[i];
        if (mod->init_func) {
            printf("Calling start func for module %s @ %p\n", mod->exported_modules[0].name.c_str(), mod->init_func);
            mod->init_func(0, nullptr, nullptr);
            if (mod->exported_modules.size())
                printf("Initialized module %s\n", mod->exported_modules[0].name.c_str());   // Use the name of the first exported module just to print something
            else printf("Initialized unnamed module\n");   // Probably won't ever happen?
        }

        if (mod->filename == "libSceLibcInternal.sprx") {
            // Run _malloc_init
            auto* sym = mod->findSymbolExport(Helpers::nameToNid("_malloc_init"));
            ((PS4_FUNC void(*)())(sym->ptr))();

            // Run sceLibcInternalMemoryMutexEnable
            sym = mod->findSymbolExport(Helpers::nameToNid("sceLibcInternalMemoryMutexEnable"));
            ((PS4_FUNC s64(*)())(sym->ptr))();
        }
    }

    // Dummy arguments
    std::memset(app->params.argv, 0, 33 * sizeof(char*));
    app->params.argc = 1;
    app->params.argv[0] = "/app0/eboot.bin";
    
    if (PS4::Configuration::is_vsh) {
        app->params.argc = 2;
        app->params.argv[1] = "--cold-boot";
        //params.argv[2] = "--detected-bad-power-down";
    }

    app->params.entry = (*modules)[0]->entry;

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
    : "r"(app->params.entry), "r"((u64)app->params.argc), "r"(app->params.argv[0]), "r"(&app->params), "r"(exitFunc)
    : "rax", "rsi", "rdi"
    );

    // Unreachable
    return nullptr;
}

void App::run() {
    Helpers::debugAssert(modules.size(), "App::run: no modules loaded\n");

    // Run app
    log("Running app\n");

    // Initialize system VideoOut port (used by VSH)
    // It looks like VSH expects this port to have handle 2.
    // We can change it because the first 0x100 handles are reserved (see SceObj.cpp) so handle 2 will never be allocated.
    if (PS4::Configuration::is_vsh || PS4::Configuration::force_init_sce_compositor) {
        using namespace PS4::OS::Libs::Kernel;
        using namespace PS4::OS::Libs::SceVideoOut;

        auto handle = sceVideoOutOpen(0, 0, 0, nullptr);
        auto* port = PS4::OS::find<SceVideoOutPort>(handle);
        port->handle = 2;

        constexpr u32 sce_composite_color_width = 1280;
        constexpr u32 sce_composite_color_height = 720;
        constexpr u32 color_target_size = sce_composite_color_width * sce_composite_color_height * 4;

        // SceVideoOut::bufs[0].base will be patched by libSceComposite, but we need to allocate a buffer for the first frame flip
        sce_composite_color_target_addr = nullptr;
        void* dmem_addr;
        // TODO: User proper flags when I emulate them
        sceKernelAllocateMainDirectMemory(color_target_size, 0x1000, 0, &dmem_addr);
        sceKernelMapDirectMemory(&sce_composite_color_target_addr, color_target_size, 0, 0, dmem_addr, 0x1000);

        if (sce_composite_color_target_addr == nullptr)
            Helpers::panic("Failed to allocate libSceComposite color target\n");

        SceVideoOutBufferAttribute attrib = {};
        attrib.pixel_format = SCE_VIDEO_OUT_PIXEL_FORMAT_A8R8G8B8_SRGB;
        attrib.tiling_mode = 0; // linear
        attrib.aspect_ratio = 0;
        attrib.width = sce_composite_color_width;
        attrib.height = sce_composite_color_height;
        attrib.pitch_in_pixels = sce_composite_color_width;
        attrib.option = 0;
        attrib._reserved0 = 0;
        attrib._reserved1 = 0;

        void* addrs[1] = { sce_composite_color_target_addr };
        sceVideoOutRegisterBuffers(2, 0, addrs, 1, &attrib);
    }

    // Create main thread
    auto& main_thread = PS4::OS::Thread::createThread("main", (PS4::OS::Thread::ThreadStartFunc)initAndJumpToEntry, this);

    void* val;
    PS4::OS::Thread::joinThread(main_thread, &val);
}

std::tuple<u8*, size_t, size_t> App::getTLSImage(u32 modid) {
    Helpers::debugAssert(modules.size(), "App::getTLSImage: no modules loaded\n");
    // Find module that contains the TLS block with id == modid
    for (auto& mod : modules) {
        if (mod->tls_modid == modid) {
            return { (u8*)mod->tls_vaddr, (size_t)mod->tls_filesz, (size_t)mod->tls_memsz };
        }
    }

    Helpers::panic("Could not find TLS image with id %d\n", modid);    
}

std::shared_ptr<Module> App::getHLEModule() {
    for (auto& m : modules) {
        if (m->filename == "HLE")
            return m;
    }
    Helpers::panic("App::getHLEModule: no HLE module found\n");
}

std::shared_ptr<Module> App::findModule(s32 modid) {
    for (auto& m : modules) {
        if (m->modid == modid)
            return m;
    }
    Helpers::panic("App::findModule: no module found with id %d\n", modid);
}

std::shared_ptr<Module> App::findModuleByName(const std::string& name) {
    for (auto& m : modules) {
        if (m->filename == name)
            return m;
    }
    Helpers::panic("App::findModuleByName: no module found with name \"%s\"\n", name.c_str());
}

std::shared_ptr<Module> App::findModuleByAddress(void* addr) {
    for (auto& m : modules) {
        if (Helpers::inRangeSized<uptr>((uptr)addr, (uptr)m->base_address, (uptr)m->size))
            return m;
    }
    return nullptr;
    Helpers::panic("App::findModuleByAddress: no module found at address %p\n", addr);
}

// Return true from the callback when you need to stop
void App::forEachModule(std::function<bool(std::shared_ptr<Module>)> func) {
    for (auto& m : modules) {
        bool stop = func(m);
        if (stop)
            break;
    }
}