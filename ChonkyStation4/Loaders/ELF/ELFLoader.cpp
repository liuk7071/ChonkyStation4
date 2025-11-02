#include "ELFLoader.hpp"

#ifdef _WIN32
#include <windows.h>
#endif


using namespace ELFIO;

Module ELFLoader::load(const fs::path& path) {
    elfio elf;
    Module module;

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
        if (type != PT_LOAD && type != PT_DYNAMIC) continue;
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
        case PT_LOAD: {
            loadSegment(*seg, module);
            break;
        }

        case PT_DYNAMIC: {
            module.dynamic_ptr = loadSegment(*seg, module);
            break;
        }

        // Contains data for dynamic linking (PT_DYNAMIC) (i.e. string tables and symbol info)
        case PT_SCE_DYNLIBDATA: {
            module.dynamic_data.resize(seg->get_file_size());
            std::memcpy(module.dynamic_data.data(), seg->get_data(), seg->get_file_size());
            break;
        }
        }
    }

    // Load dynamic linking data
    for (Elf64_Dyn* dyn = (Elf64_Dyn*)module.dynamic_ptr; dyn->d_tag != DT_NULL; dyn++) {
        log("%s\n", dump::str_dynamic_tag(dyn->d_tag).c_str());

        switch (dyn->d_tag) {
        case DT_SCE_NEEDED_MODULE: {
            auto& module_info = required_modules.emplace_back();
            module_info.load(dyn->d_un.d_val, module.dyn_str_table);
            log("Required module %s\n", module_info.name.c_str());
            break;
        }

        case DT_SCE_IMPORT_LIB: {
            auto& lib = required_libs.emplace_back();
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

    // Print symbols
    if (!module.sym_table) log("WARNING: NO SYMBOL TABLE!\n");
    for (Elf64_Sym* sym = module.sym_table; (u8*)sym < (u8*)module.sym_table + module.sym_table_size; sym++) {
        const std::string sym_name = module.dyn_str_table + sym->st_name;
        //log("* %s\n", sym_name.c_str());
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

    DWORD old_flags;
    if (!VirtualProtect((LPVOID*)ptr, seg.get_memory_size(), get_perms(seg.get_flags()), &old_flags)) {
        Helpers::panic("ELFLoader: VirtualProtect failed\n");
    }

    return (void*)((u8*)ptr);
#else
    Helpers::panic("Unsupported platform\n");
#endif
}