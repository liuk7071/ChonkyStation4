#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceNet {

void init(Module& module);

s32* PS4_FUNC sceNetErrnoLoc();
u16 PS4_FUNC sceNetHtons(u16 host16);
u32 PS4_FUNC sceNetHtonl(u32 host32);
u32 PS4_FUNC sceNetNtohl(u32 net32);
u16 PS4_FUNC sceNetNtohs(u16 net16);
const char* PS4_FUNC sceNetInetNtop(int af, const void* src, char* dst, u32 size);

}   // End namespace PS4::OS::Libs::SceNet