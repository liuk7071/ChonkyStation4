#pragma once

#include <Common.hpp>


class Module;

namespace PS4::Loader::ELF {

void patchCode(Module& module, u8* ptr, size_t size);

}   // End namespace Loader::ELF