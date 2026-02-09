#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceNet {

void init(Module& module);

s32* PS4_FUNC sceNetErrnoLoc();
u16 PS4_FUNC sceNetHtons(u16 host16);
u32 PS4_FUNC sceNetHtonl(u32 host32);

}   // End namespace PS4::OS::Libs::SceNet