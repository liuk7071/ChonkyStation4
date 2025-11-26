#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::ScePad {

void init(Module& module);

s32 PS4_FUNC scePadInit();

}   // End namespace PS4::OS::Libs::ScePad