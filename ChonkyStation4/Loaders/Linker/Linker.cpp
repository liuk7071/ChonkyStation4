#include "Linker.hpp"
#include <Logger.hpp>
#include <Loaders/ELF/ELFLoader.hpp>
#include <Loaders/App.hpp>
#include <OS/HLE.hpp>

#include <memory>


namespace PS4::Loader::Linker {

MAKE_LOG_FUNCTION(log, loader_linker);

using namespace ELFIO;

// Unresolved symbols are stubbed to call this function
void PS4_FUNC unresolvedSymbol(const char* sym_name) {
    printf("Linker: Called unresolved symbol %s\n", sym_name);
    //exit(0);
    std::_Exit(0);
}

void* generateTrampolineForUnresolvedSymbol(App& app, const std::string& sym_name) {
    using namespace Xbyak::util;

    app.unresolved_symbols.push_back(sym_name);
    const char* str_ptr = app.unresolved_symbols.back().c_str();

    auto code = std::make_unique<Xbyak::CodeGenerator>(128);
    
    code->mov(rdi, (u64)str_ptr);
    code->mov(rax, (u64)&unresolvedSymbol);
    code->jmp(rax);

    void* ptr = (void*)code->getCode();
    app.unresolved_symbol_handlers.push_back(std::move(code));
    return ptr;
}

App loadAndLink(const fs::path& path) {
    ELFLoader loader;
    App app;

    // Load main executable module
    app.modules.push_back(loader.load(path));

    // Build HLE module
    app.modules.push_back(PS4::OS::HLE::buildHLEModule());

    // Relocate
    doRelocations(app);

    log("App has %d unresolved symbols\n", app.unresolved_symbols.size());
    log("App linked successfully\n");

    return app;
}

void doRelocations(App& app) {
    // Iterate over the relocation tables of all loaded modules
    auto relocate = [](App& app, Module& module, Elf64_Rela* reloc_table, size_t reloc_table_size) {
        for (Elf64_Rela* rela = reloc_table; (u8*)rela < (u8*)reloc_table + reloc_table_size; rela++) {
            const auto type = ELF64_R_TYPE(rela->r_info);
            const auto base = module.base_address;
            //log("Relocation type: %x\n", type);
            //log("Relocation offs: 0x%llx\n", rela->r_offset);

            switch (type) {
            case R_X86_64_RELATIVE: {
                *(u64*)((u8*)base + rela->r_offset) = (u64)base + rela->r_addend;
                break;
            }

            case R_X86_64_GLOB_DAT:
            case R_X86_64_JUMP_SLOT:
            case R_X86_64_64: {
                auto addend = rela->r_addend;
                if (type != R_X86_64_64) addend = 0;
                auto* sym = &module.sym_table[ELF64_R_SYM(rela->r_info)];
                auto bind = ELF_ST_BIND(sym->st_info);
                const std::string sym_name = module.dyn_str_table + sym->st_name;

                // Is this a local symbol?
                if (bind == STB_LOCAL) {
                    *(u64*)((u8*)base + rela->r_offset) = (u64)base + sym->st_value;
                    break;
                }

                auto tokens = Helpers::split(sym_name, "#");
                void* ptr = nullptr;
                if (tokens.size() == 3) {
                    // Find library and module
                    auto* lib = module.findLibrary(tokens[1]);
                    auto* mod = module.findModule(tokens[2]);
                    Helpers::debugAssert(lib, "Linker: could not find library for symbol %s\n", sym_name.c_str());
                    Helpers::debugAssert(mod, "Linker: could not find module for symbol %s\n", sym_name.c_str());

                    // Iterate over the list of exported symbols in the app to try to find it
                    for (auto& m : app.modules) {
                        Symbol* exported_sym = m.findSymbolExport(tokens[0], lib->name, mod->name);
                        if (exported_sym) {
                            log("* Resolved symbol %s as %s (%s)\n", sym_name.c_str(), exported_sym->name.c_str(), exported_sym->lib.c_str());
                            ptr = exported_sym->ptr;
                            break;
                        }
                    }

                    // If we couldn't resolve the symbol, make it point to the unresolved symbol handler
                    if (!ptr) {
                        log("* Could not resolve symbol %s (%s)\n", sym_name.c_str(), lib->name.c_str());
                        ptr = generateTrampolineForUnresolvedSymbol(app, sym_name);
                    }
                } else {
                    // Symbol is not a nid
                    log("* Could not resolve non-nid symbol %s\n", sym_name.c_str());
                    break;
                }
                

                *(u64*)((u8*)base + rela->r_offset) = (u64)ptr + addend;
                break;
            }

            case R_X86_64_DTPMOD64: {
                *(u64*)((u8*)base + rela->r_offset) = module.tls_modid;
                break;
            }

            default: {
                Helpers::panic("Unhandled relocation type 0x%x\n", type);
            }
            }
        }
    };

    for (auto& module : app.modules) {
        relocate(app, module, module.reloc_table, module.reloc_table_size);
        relocate(app, module, module.jmp_reloc_table, module.jmp_reloc_table_size);
    }
}

void loadAndLinkLib(App& app, const fs::path& path, bool is_partial_lle_module, Module* hle_module) {
    ELFLoader loader;
    app.modules.push_back(loader.load(path, is_partial_lle_module, hle_module));
    doRelocations(app);
}

} // End namespace Loader::Linker