#pragma once

#include <Common.hpp>
#include <Logger.hpp>
#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>
#include <Loaders/Module.hpp>

#include <unordered_map>


// PS4 specific
static constexpr u32 PT_SCE_DYNLIBDATA  = 0x61000000;
static constexpr u32 PT_SCE_PROCPARAM   = 0x61000001;
static constexpr u32 PT_SCE_RELRO       = 0x61000010;

static constexpr u32 DT_SCE_MODULE_INFO     = 0x6100000d;
static constexpr u32 DT_SCE_NEEDED_MODULE   = 0x6100000f;
static constexpr u32 DT_SCE_EXPORT_LIB      = 0x61000013;
static constexpr u32 DT_SCE_IMPORT_LIB      = 0x61000015;
static constexpr u32 DT_SCE_JMPREL          = 0x61000029;
static constexpr u32 DT_SCE_PLTRELSZ        = 0x6100002d;
static constexpr u32 DT_SCE_RELA            = 0x6100002f;
static constexpr u32 DT_SCE_RELASZ          = 0x61000031;
static constexpr u32 DT_SCE_STRTAB          = 0x61000035;
static constexpr u32 DT_SCE_SYMTAB          = 0x61000039;
static constexpr u32 DT_SCE_SYMTABSZ        = 0x6100003f;

static constexpr u32 SELF_MAGIC = 0x1d3d154f;

class ELFLoader {
public:
    ELFLoader() {}

    typedef struct {
        s64 d_tag;
        union {
            u64 d_val;
            u64 d_ptr;
        } d_un;
    } Elf64_Dyn;

    struct SELFHeader {
        u32 magic;
        u32 unknown1;
        u8  category;
        u8  program_type;
        u16 padding1;
        u16 header_size;
        u16 metadata_size;
        u32 file_size;
        u32 padding2;
        u16 n_segments;
        u16 unknown2;
        u32 padding3;
    };

    struct SELFSegment {
        u64 flags;
        u64 offset;
        u64 encrypted_compressed_size;
        u64 decrypted_decompressed_size;
    };

    struct Elf64_Ehdr {
        u32 e_magic;
        u8  e_class;
        u8  e_data;
        u8  e_curver;
        u8  e_os_abi;
        u64 e_abi_ver;
        u16 e_type;
        u16 e_machine;
        u32 e_version;
        u64 e_entry;
        u64 e_phoff;
        u64 e_shoff;
        u32 e_flags;
        u16 e_ehsize;
        u16 e_phentsize;
        u16 e_phnum;
        u16 e_shentsize;
        u16 e_shnum;
        u16 e_shstrndx;
    };

    struct Elf64_Phdr {
        u32 p_type;
        u32 p_flags;
        u64 p_offset;
        u64 p_vaddr;
        u64 p_paddr;
        u64 p_filesz;
        u64 p_memsz;
        u64 p_align;
    };
    
    struct Elf64_Shdr {
        u32 sh_name;
        u32 sh_type;
        u64 sh_flags;
        u64 sh_addr;
        u64 sh_offset;
        u64 sh_size;     
        u32 sh_link;
        u32 sh_info;
        u64 sh_addralign;
        u64 sh_entsize;
    };

    static inline std::unordered_map<u64, std::string> segment_type_string = {
        { ELFIO::PT_LOAD,    "PT_LOAD           " },
        { ELFIO::PT_TLS,     "PT_TLS            " },
        { ELFIO::PT_DYNAMIC, "PT_DYNAMIC        " },
        { ELFIO::PT_INTERP,  "PT_INTERP         " },
        { PT_SCE_DYNLIBDATA, "PT_SCE_DYNLIBDATA " },
        { PT_SCE_PROCPARAM,  "PT_SCE_PROCPARAM  " },
        { PT_SCE_RELRO,      "PT_SCE_RELRO      " },
    };

    std::shared_ptr<Module> load(const fs::path& path, bool is_partial_lle_module = false, std::shared_ptr<Module> hle_module = nullptr);

private: 
    MAKE_LOG_FUNCTION(log, loader_elf);
    
    void* loadSegment(ELFIO::segment& seg, std::shared_ptr<Module> module, u8* ptr = nullptr, bool do_patch = true, bool zero_fill = true);
};