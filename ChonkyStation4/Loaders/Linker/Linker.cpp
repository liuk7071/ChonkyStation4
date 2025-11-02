#include "Linker.hpp"
#include <Logger.hpp>
#include <Loaders/ELF/ELFLoader.hpp>


namespace Loader::Linker {

MAKE_LOG_FUNCTION(log, loader_linker);

using namespace ELFIO;

// Unresolved symbols are stubbed to call this function
__attribute__((sysv_abi)) void unresolvedSymbol() {
    // TODO: How do I know which symbol I am?
    printf("Linker: Called unresolved symbol\n");
    exit(0);
}

App loadAndLink(const fs::path& path) {
    ELFLoader loader;
    App app;

    // Load main executable module
    app.modules.push_back(loader.load(path));

    // TODO: Load HLE modules
    // TODO: Load LLE modules
    
    // Iterate over the relocation tables of all loaded modules
    auto relocate = [](Module& module, Elf64_Rela* reloc_table, size_t reloc_table_size) {
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
                *(u64*)((u8*)base + rela->r_offset) = (u64)&unresolvedSymbol;
                break;
            }

            default: {
                Helpers::panic("Unhandled relocation type 0x%x\n", type);
            }
            }
        }
    };

    for (auto& module : app.modules) {
        relocate(module, module.reloc_table, module.reloc_table_size);
        relocate(module, module.jmp_reloc_table, module.jmp_reloc_table_size);
    }

    return app;
}
} // End namespace Loader::Linker