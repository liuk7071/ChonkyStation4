#pragma once

#include <Common.hpp>
#include <elfio/elfio.hpp>
#include <Loaders/Symbol.hpp>


namespace PS4::Loader {

    static std::string encodeID(u16 id) {
        static constexpr char* charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
        std::string res;
        
        if (id < 64) {
            res += charset[id];
        } else {
            Helpers::panic("encodeID: unimplemented ID >= 64\n", id);
        }

        return res;
    }

}   // End namespace Loader

class Module {
public:
    void* base_address = nullptr;  // Base address of the module in memory
    void* entry = nullptr;

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

    std::vector<ModuleInfo> required_modules;
    std::vector<LibraryInfo> required_libs;
    std::vector<Symbol> exported_symbols;

    ModuleInfo* findModule(const std::string& id) {
        for (auto& module : required_modules) {
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