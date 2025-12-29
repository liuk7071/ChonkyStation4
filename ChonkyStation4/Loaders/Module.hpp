#pragma once

#include <Common.hpp>
#include <Logger.hpp>
#include <elfio/elfio.hpp>
#include <Loaders/Symbol.hpp>
#include <xbyak/xbyak.h>
#include <deque>
#include <memory>


namespace PS4::Loader {

MAKE_LOG_FUNCTION(unimpl, unimplemented);

using InitFunc = PS4_FUNC int (*)(size_t args, const void* argp, void* param);

struct SceKernelMemParam {
    u64 size;
    u64* extended_page_table;
    u64* flexible_memory_size;
    u8* extended_memory_1;
    u64* extended_gpu_page_table;
    u8* extended_memory_2;
    u64* extended_cpu_page_table;
};

struct SceProcParam {
    u64 size;
    u32 magic;
    u32 entry_count;
    u64 sdk_version;
    char* process_name;
    char* main_thread_name;
    u32* main_thread_prio;
    u32* main_thread_stack_size;
    void* libc_param;
    SceKernelMemParam* mem_param;
    void* fs_param;
    u32* process_preload_enable;
    u64 unknown1;
};

inline std::deque<std::string> stubbed_symbols;
inline std::deque<s32> stubbed_symbol_ret_vals;
inline std::vector<std::unique_ptr<Xbyak::CodeGenerator>> stubbed_symbol_handlers;

static std::string encodeID(u16 id) {
    const char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
    std::string res;
    
    if (id < 64) {
        res += charset[id];
    } else if (id < 0x1000) {
        res += charset[(id >> 6) & 0x3fu];
        res += charset[id & 0x3fu];
    } else {
        res += charset[(id >> 12) & 0x3fu];
        res += charset[(id >> 6) & 0x3fu];
        res += charset[id & 0x3fu];
    }

    return res;
}

static s32 PS4_FUNC stubbedSymbol(const char* sym_name, s32* ret_val) {
    PS4::Loader::unimpl("%s(): TODO (returning %d)\n", sym_name, *ret_val);
    return *ret_val;
}

}   // End namespace Loader

class Module {
public:
    void* base_address = nullptr;  // Base address of the module in memory
    void* entry = nullptr;
    std::string filename;

    struct ModuleInfo {
        void load(u64 val, char* str_table_ptr) {
            const u32 str_table_offs = (val & 0xffffffff);
            const u16 id = (val >> 48) & 0xffff;
            name = str_table_ptr + str_table_offs;
            this->id = PS4::Loader::encodeID(id);
        }

        std::string name;
        std::string id;
    };

    struct LibraryInfo {
        void load(u64 val, char* str_table_ptr) {
            const u32 str_table_offs = (val & 0xffffffff);
            const u16 id = (val >> 48) & 0xffff;
            name = str_table_ptr + str_table_offs;
            this->id = PS4::Loader::encodeID(id);
        }

        std::string name;
        std::string id;
    };

    PS4::Loader::InitFunc init_func = nullptr;
    std::vector<ModuleInfo> required_modules;
    std::vector<LibraryInfo> required_libs;
    std::vector<ModuleInfo> exported_modules;
    std::vector<LibraryInfo> exported_libs;
    std::vector<Symbol> exported_symbols;
    u64 tls_vaddr = 0;
    u64 tls_filesz = 0;
    u64 tls_memsz = 0;
    u32 tls_modid = 0;
    u64 proc_param_ptr = 0;

    ModuleInfo* findModule(const std::string& id) {
        for (auto& module : required_modules) {
            if (module.id == id)
                return &module;
        }
        for (auto& module : exported_modules) {
            if (module.id == id)
                return &module;
        }
        return nullptr;
    }

    LibraryInfo* findLibrary(const std::string& id) {
        for (auto& lib : required_libs) {
            if (lib.id == id)
                return &lib;
        }
        for (auto& lib : exported_libs) {
            if (lib.id == id)
                return &lib;
        }
        return nullptr;
    }

    Symbol* findSymbolExport(const std::string& name, const std::string& lib, const std::string& module) {
        for (auto& sym : exported_symbols) {
            if ((sym.nid == name) && (sym.lib == lib) && (sym.module == module))
                return &sym;
        }
        return nullptr;
    }

    void addSymbolExport(const std::string& nid, const std::string& name, const std::string& lib, const std::string& module, void* ptr) {
        Symbol sym;
        sym.nid = nid;
        sym.name = name;
        sym.lib = lib;
        sym.module = module;
        sym.ptr = ptr;
        exported_symbols.push_back(sym);
    }

    void addSymbolStub(const std::string& nid, const std::string& name, const std::string& lib, const std::string& module, s32 ret_val = 0) {
        // TODO: This code is duplicated from Linker.cpp
        using namespace Xbyak::util;

        PS4::Loader::stubbed_symbols.push_back(name);
        PS4::Loader::stubbed_symbol_ret_vals.push_back(ret_val);
        const char* str_ptr = PS4::Loader::stubbed_symbols.back().c_str();
        s32* ret_val_ptr = &PS4::Loader::stubbed_symbol_ret_vals.back();

        auto code = std::make_unique<Xbyak::CodeGenerator>(128);
        
        code->mov(rdi, (u64)str_ptr);
        code->mov(rsi, (u64)ret_val_ptr);
        code->mov(rax, (u64)&PS4::Loader::stubbedSymbol);
        code->jmp(rax);

        void* ptr = (void*)code->getCode();
        PS4::Loader::stubbed_symbol_handlers.push_back(std::move(code));
        addSymbolExport(nid, name, lib, module, ptr);
    }

    std::vector<u8> dynamic_tags;
    std::vector<u8> dynamic_data;

    // Dynamic segment info
    char* dyn_str_table = nullptr;
    ELFIO::Elf64_Sym* sym_table = nullptr;
    size_t sym_table_size = 0;
    ELFIO::Elf64_Rela* reloc_table = nullptr;
    size_t reloc_table_size = 0;
    ELFIO::Elf64_Rela* jmp_reloc_table = nullptr;
    size_t jmp_reloc_table_size = 0;
};