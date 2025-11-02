#pragma once

#include <Common.hpp>
#include <elfio/elfio.hpp>


class Module {
public:
    void* base_address = nullptr;  // Base address of the module in memory
    void* entry = nullptr;

    void* dynamic_ptr = nullptr;    // Pointer to the PT_DYNAMIC segment in memory
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