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

static constexpr u32 DT_SCE_NEEDED_MODULE   = 0x6100000f;
static constexpr u32 DT_SCE_IMPORT_LIB      = 0x61000015;
static constexpr u32 DT_SCE_JMPREL          = 0x61000029;
static constexpr u32 DT_SCE_PLTRELSZ        = 0x6100002d;
static constexpr u32 DT_SCE_RELA            = 0x6100002f;
static constexpr u32 DT_SCE_RELASZ          = 0x61000031;
static constexpr u32 DT_SCE_STRTAB          = 0x61000035;
static constexpr u32 DT_SCE_SYMTAB          = 0x61000039;
static constexpr u32 DT_SCE_SYMTABSZ        = 0x6100003f;

class ELFLoader {
public:
    ELFLoader() {}

    typedef struct {
        s64 d_tag;
        union {
            u64 d_val;
            u64  d_ptr;
        } d_un;
    } Elf64_Dyn;

    struct ModuleInfo {
        void load(u64 val, char* str_table_ptr) {
            u32 str_table_offs = (val & 0xffffffff);
            name = str_table_ptr + str_table_offs;
        }

        std::string name;
    };

    struct LibraryInfo {
        void load(u64 val, char* str_table_ptr) {
            u32 str_table_offs = (val & 0xffffffff);
            name = str_table_ptr + str_table_offs;
        }

        std::string name;
    };

    std::vector<ModuleInfo> required_modules;
    std::vector<LibraryInfo> required_libs;

    static inline std::unordered_map<u64, std::string> segment_type_string = {
        { ELFIO::PT_LOAD,    "PT_LOAD           " },
        { ELFIO::PT_TLS,     "PT_TLS            " },
        { ELFIO::PT_DYNAMIC, "PT_DYNAMIC        " },
        { ELFIO::PT_INTERP,  "PT_INTERP         " },
        { PT_SCE_DYNLIBDATA, "PT_SCE_DYNLIBDATA " },
        { PT_SCE_PROCPARAM,  "PT_SCE_PROCPARAM  " },
        { PT_SCE_RELRO,      "PT_SCE_RELRO      " },
    };

    Module load(const fs::path& path);

private: 
    MAKE_LOG_FUNCTION(log, loader_elf);
    
    void* loadSegment(ELFIO::segment& seg, Module& module);
};