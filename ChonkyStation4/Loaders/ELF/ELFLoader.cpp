#include "ELFLoader.hpp"
#include "CodePatcher.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif


using namespace ELFIO;

static int tls_modid = 0;

Module ELFLoader::load(const fs::path& path) {
    elfio elf;
    Module module;
    module.filename = path.filename().generic_string();

    auto str = path.generic_string();
    if (!elf.load(str.c_str())) {
        Helpers::panic("Couldn't load ELF %s:\n%s\n", str.c_str(), elf.validate().c_str());
    }

    log("Loading ELF %s\n", str.c_str());
    log("* %d segments\n", elf.segments.size());

    // Iterate over segments to find the total size we need to allocate
    u64 min_vaddr = -1ull;
    u64 max_vaddr = 0;
    for (auto& seg : elf.segments) {
        const auto type = seg->get_type();
        if (type != PT_LOAD) continue;
        u64 start = seg->get_virtual_address();
        u64 end   = start + seg->get_memory_size();
        if (start < min_vaddr) min_vaddr = start;
        if (end   > max_vaddr) max_vaddr = end;
    }
    const size_t total_size = max_vaddr - min_vaddr;
    log("* Total size of ELF: %d bytes (%f KB)\n", total_size, total_size / 1024.0f);

    // Allocate memory
#ifdef _WIN32
    module.base_address = VirtualAlloc(nullptr, total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Helpers::debugAssert(module.base_address, "ELFLoader: VirtualAlloc failed");
#else
    Helpers::panic("Unsupported platform\n");
#endif

    log("* Base address of ELF: 0x%016llx\n", (u64)module.base_address);

    module.entry = (void*)((u8*)module.base_address + elf.get_entry());
    log("* Entry: 0x%016llx\n", (u64)module.entry);

    for (int i = 0; i < elf.segments.size(); i++) {
        auto seg = elf.segments[i];
        
        std::string type_string;
        if (segment_type_string.contains(seg->get_type()))
            type_string = segment_type_string[seg->get_type()];
        else
            type_string = std::format("0x{:08x}", seg->get_type());

        log("* Segment %d type %s: 0x%016llx -> 0x%016llx\n", i, type_string.c_str(), seg->get_virtual_address(), seg->get_virtual_address() + seg->get_memory_size());
        switch (seg->get_type()) {
        case PT_LOAD:
        case PT_SCE_RELRO: {
            loadSegment(*seg, module);
            break;
        }

        case PT_DYNAMIC: {
            module.dynamic_tags.resize(seg->get_file_size());
            std::memcpy(module.dynamic_tags.data(), seg->get_data(), seg->get_file_size());
            break;
        }

        // Contains data for dynamic linking (PT_DYNAMIC) (i.e. string tables and symbol info)
        case PT_SCE_DYNLIBDATA: {
            module.dynamic_data.resize(seg->get_file_size());
            std::memcpy(module.dynamic_data.data(), seg->get_data(), seg->get_file_size());
            break;
        }

        case PT_SCE_PROCPARAM: {
            module.proc_param_ptr = seg->get_virtual_address() + (u64)module.base_address;
            break;
        }

        // TLS info
        case PT_TLS: {
            module.tls_vaddr = seg->get_virtual_address() + (u64)module.base_address;
            module.tls_size = seg->get_file_size();
            module.tls_modid = tls_modid++;
            break;
        }
        }
    }

    // Load dynamic linking data
    for (Elf64_Dyn* dyn = (Elf64_Dyn*)module.dynamic_tags.data(); dyn->d_tag != DT_NULL; dyn++) {
        log("%s\n", dump::str_dynamic_tag(dyn->d_tag).c_str());

        switch (dyn->d_tag) {
        case DT_INIT: {
            module.init_func = (PS4::Loader::InitFunc)(dyn->d_un.d_ptr + (u64)module.base_address);
            break;
        }

        case DT_SCE_MODULE_INFO: {
            auto& module_info = module.exported_modules.emplace_back();
            module_info.load(dyn->d_un.d_val, module.dyn_str_table);
            log("Exported module %s\n", module_info.name.c_str());
            break;
        }

        case DT_SCE_NEEDED_MODULE: {
            auto& module_info = module.required_modules.emplace_back();
            module_info.load(dyn->d_un.d_val, module.dyn_str_table);
            log("Required module %s\n", module_info.name.c_str());
            break;
        }

        case DT_SCE_EXPORT_LIB: {
            auto& lib = module.exported_libs.emplace_back();
            lib.load(dyn->d_un.d_val, module.dyn_str_table);
            log("Exported library %s\n", lib.name.c_str());
            break;
        }

        case DT_SCE_IMPORT_LIB: {
            auto& lib = module.required_libs.emplace_back();
            lib.load(dyn->d_un.d_val, module.dyn_str_table);
            log("Imported library %s\n", lib.name.c_str());
            break;
        }

        case DT_SCE_JMPREL: {
            module.jmp_reloc_table = (Elf64_Rela*)(module.dynamic_data.data() + dyn->d_un.d_ptr);
            break;
        }

        case DT_SCE_PLTRELSZ: {
            module.jmp_reloc_table_size = dyn->d_un.d_val;
            break;
        }

        case DT_SCE_RELA: {
            module.reloc_table = (Elf64_Rela*)(module.dynamic_data.data() + dyn->d_un.d_ptr);
            break;
        }

        case DT_SCE_RELASZ: {
            module.reloc_table_size = dyn->d_un.d_val;
            break;
        }

        case DT_SCE_STRTAB: {   // Sets the pointer of the string table
            module.dyn_str_table = (char*)module.dynamic_data.data() + dyn->d_un.d_ptr;
            break;
        }

        case DT_SCE_SYMTAB: {
            module.sym_table = (Elf64_Sym*)(module.dynamic_data.data() + dyn->d_un.d_ptr);
            break;
        }

        case DT_SCE_SYMTABSZ: {
            module.sym_table_size = dyn->d_un.d_val;
            break;
        }
        }
    }

    // Export symbols
    if (!module.sym_table) log("WARNING: NO SYMBOL TABLE!\n");
    for (Elf64_Sym* sym = module.sym_table; (u8*)sym < (u8*)module.sym_table + module.sym_table_size; sym++) {
        auto bind = ELF_ST_BIND(sym->st_info);
        const std::string sym_name = module.dyn_str_table + sym->st_name;

        // Export STB_GLOBAL and STB_WEAK symbols (not local symbols) that have st_value != 0
        if (bind == STB_LOCAL || sym->st_value == 0) continue;

        // Find library and module
        auto tokens = Helpers::split(sym_name, "#");
        Helpers::debugAssert(tokens.size() == 3, "Linker: invalid symbol %s\n", sym_name.c_str());
        auto* lib = module.findLibrary(tokens[1]);
        auto* mod = module.findModule(tokens[2]);
        Helpers::debugAssert(lib, "Linker: could not find library for symbol %s\n", sym_name.c_str());
        Helpers::debugAssert(mod, "Linker: could not find module for symbol %s\n", sym_name.c_str());

        // Export symbol
        module.addSymbolExport(tokens[0], tokens[0], lib->name, mod->name, (void*)((u64)module.base_address + sym->st_value));
        log("* Exported symbol %s (%s)\n", tokens[0].c_str(), lib->name.c_str());
    }

    // Print modules
    log("Loaded modules:\n");
    for (auto& module : module.required_modules) {
        log("* %s: %s\n", module.id.c_str(), module.name.c_str());
    }
    log("Exported modules:\n");
    for (auto& module : module.exported_modules) {
        log("* %s: %s\n", module.id.c_str(), module.name.c_str());
    }

    // Print libraries
    log("Loaded libraries:\n");
    for (auto& lib : module.required_libs) {
        log("* %s: %s\n", lib.id.c_str(), lib.name.c_str());
    }
    log("Exported libraries:\n");
    for (auto& lib : module.exported_libs) {
        log("* %s: %s\n", lib.id.c_str(), lib.name.c_str());
    }

    // Fix permissions for each segment
    for (auto& seg : elf.segments) {
        if (seg->get_type() != PT_LOAD && seg->get_type() != PT_SCE_RELRO) continue;

#ifdef _WIN32
        // Set permissions
        auto get_perms = [](u32 flags) -> u64 {
            bool r = flags & PF_R;
            bool w = flags & PF_W;
            bool x = flags & PF_X;
            if (r && !w && !x) return PAGE_READONLY;
            else if (r && w && !x) return PAGE_READWRITE;
            else if (r && !w && x) return PAGE_EXECUTE_READ;
            else if (r && w && x) return PAGE_EXECUTE_READWRITE;
            else Helpers::panic("ELFLoader: invalid flags 0x%08x\n", flags);
        };

        const u8* ptr = (u8*)module.base_address + seg->get_virtual_address();

        // TODO: To apply permissions properly (get_perms(seg->get_flags())) we need to apply them AFTER relocations happen.
        //       Doing that requires restructuring some code. Stub to RWX.
        DWORD old_flags;
        if (!VirtualProtect((LPVOID*)ptr, seg->get_memory_size(), PAGE_EXECUTE_READWRITE, &old_flags)) {
            Helpers::panic("ELFLoader: VirtualProtect failed\n");
        }
#else
        Helpers::panic("Unsupported platform\n");
#endif

    }

    return module;
}

void* ELFLoader::loadSegment(ELFIO::segment& seg, Module& module) {
    // Load segment in host memory
    const u8* ptr = (u8*)module.base_address + seg.get_virtual_address();
    const u64 size = seg.get_memory_size();

#ifdef _WIN32
    // Copy segment data
    std::memcpy((u8*)ptr, seg.get_data(), seg.get_file_size());
    // Set the remaining memory to 0
    std::memset((u8*)ptr + seg.get_file_size(), 0, seg.get_memory_size() - seg.get_file_size());
#else
    Helpers::panic("Unsupported platform\n");
#endif

    // Apply patches if the segment is executable
    const bool x = seg.get_flags() & PF_X;
    if (x) PS4::Loader::ELF::patchCode(module, (u8*)ptr, size);

    return (void*)((u8*)ptr);
}