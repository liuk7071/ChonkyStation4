#include "ELFLoader.hpp"
#include <Logger.hpp>

#ifdef _WIN32
#include <windows.h>
#endif


MAKE_LOG_FUNCTION(log, loader_elf);

using namespace ELFIO;

void* ELFLoader::load(const fs::path& path) {
    elfio elf;

    auto str = path.generic_string();
    if (!elf.load(str.c_str())) {
        Helpers::panic("Couldn't load ELF %s:\n%s\n", str.c_str(), elf.validate().c_str());
    }

    log("Loading ELF %s\n", str.c_str());
    log("* %d segments\n", elf.segments.size());
    for (int i = 0; i < elf.segments.size(); i++) {
        auto seg = elf.segments[i];
        
        std::string type_string;
        if (segment_type_string.contains(seg->get_type()))
            type_string = segment_type_string[seg->get_type()];
        else
            type_string = std::format("0x{:08x}", seg->get_type());

        // Skip the segment if it's empty
        if (seg->get_file_size() == 0) {
            log("* Segment %d type %s: empty\n", i, type_string.c_str());
            continue;
        }
        log("* Segment %d type %s: 0x%016llx -> 0x%016llx\n", i, type_string.c_str(), seg->get_virtual_address(), seg->get_virtual_address() + seg->get_memory_size());

        switch (seg->get_type()) {
        case PT_LOAD: {
            loadSegment(*seg);
            break;
        }

        case PT_DYNAMIC: {
            dynamic_ptr = loadSegment(*seg);
            break;
        }

        // Contains data for dynamic linking (PT_DYNAMIC) (i.e. string tables)
        case PT_SCE_DYNLIBDATA: {
            dynamic_data.resize(seg->get_file_size());
            std::memcpy(dynamic_data.data(), seg->get_data(), seg->get_file_size());
            break;
        }
        }
    }

    // Load dynamic linking data
    for (Elf64_Dyn* dyn = (Elf64_Dyn*)dynamic_ptr; dyn->d_tag != DT_NULL; dyn++) {
        log("%s\n", dump::str_dynamic_tag(dyn->d_tag).c_str());

        switch (dyn->d_tag) {
        case DT_SCE_NEEDED_MODULE: {
            auto& module = required_modules.emplace_back();
            module.load(dyn->d_un.d_val, dyn_str_table);
            log("Required module %s\n", module.name.c_str());
            break;
        }

        case DT_SCE_STRTAB: {   // Sets the pointer of the string table
            dyn_str_table = (char*)dynamic_data.data() + dyn->d_un.d_ptr;
            break;
        }
        }
    }

    return nullptr;
}

void* ELFLoader::loadSegment(ELFIO::segment& seg) {
    // Load segment in host memory
    const u64 vaddr = seg.get_virtual_address();
    const u64 size = seg.get_memory_size();

#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t page_size = si.dwAllocationGranularity;
    const u64 aligned_vaddr = vaddr & ~(page_size - 1);
    const u64 offs = vaddr - aligned_vaddr;
    const u64 real_size = (size + offs + page_size - 1) & ~(page_size - 1);

    LPVOID ptr = VirtualAlloc((LPVOID)aligned_vaddr, real_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Helpers::debugAssert(ptr, "ELFLoader: VirtualAlloc failed");

    // Copy segment data
    std::memcpy((u8*)ptr + offs, seg.get_data(), seg.get_file_size());
    // Set the remaining memory to 0
    std::memset((u8*)ptr + offs + seg.get_file_size(), 0, seg.get_memory_size() - seg.get_file_size());

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
    if (!VirtualProtect(ptr, real_size, get_perms(seg.get_flags()), &old_flags)) {
        Helpers::panic("ELFLoader: VirtualProtect failed\n");
    }

    return (void*)((u8*)ptr + offs);
#else
    Helpers::panic("Unsupported platform\n");
#endif
}