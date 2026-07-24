#pragma once

#include <Common.hpp>


class Module;

namespace PS4::Loader::ELF {

void patchCode(Module& module, u8* ptr, size_t size);
void patchRedZone(u8* func_start, u8* func_end, u8* entry_patch_addr, size_t entry_patch_size, u8* exit_patch_addr, size_t exit_patch_size);

}   // End namespace Loader::ELF