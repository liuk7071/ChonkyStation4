#include "Linker.hpp"
#include <Logger.hpp>
#include <Loaders/ELF/ELFLoader.hpp>
#include <Loaders/App.hpp>

#include <memory>


namespace Loader::Linker {

MAKE_LOG_FUNCTION(log, loader_linker);

using namespace ELFIO;

// Unresolved symbols are stubbed to call this function
__attribute__((sysv_abi)) void unresolvedSymbol(const char* sym_name) {
    // TODO: How do I know which symbol I am?
    printf("Linker: Called unresolved symbol %s\n", sym_name);
    exit(0);
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

    // TODO: Load HLE modules
    // TODO: Load LLE modules
    
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

            case R_X86_64_JUMP_SLOT: {
                auto* sym = &module.sym_table[ELF64_R_SYM(rela->r_info)];
                const std::string sym_name = module.dyn_str_table + sym->st_name;
                log("* Resolving symbol %s\n", sym_name.c_str());
                *(u64*)((u8*)base + rela->r_offset) = (u64)generateTrampolineForUnresolvedSymbol(app, sym_name);
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

    return app;
}

} // End namespace Loader::Linker