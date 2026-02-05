#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceRandom {

void init(Module& module);

s32 PS4_FUNC sceRandomGetRandomNumber(void* buf, size_t size);

}   // End namespace PS4::OS::Libs::SceRandom