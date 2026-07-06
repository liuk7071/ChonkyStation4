#pragma once

#include <Common.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNet {

void init(Module& module);

static constexpr s32 SCE_NET_AF_INET = 2;
static constexpr s32 SCE_NET_SOCK_STREAM = 1;
static constexpr s32 SCE_NET_SOCK_DGRAM = 2;
static constexpr s32 SCE_NET_SOCK_RAW = 3;
static constexpr s32 SCE_NET_SOCK_DGRAM_P2P = 6;
static constexpr s32 SCE_NET_SOCK_STREAM_P2P = 0;

using SceNetId = s32;
using SceNetSaFamily = u8;
using SceNetInPort = u16;
using SceNetInAddr = u32;
using SceNetSocklen = u32;

struct SceNetSockaddr {
    u8 len;
    SceNetSaFamily family;
    char data[14];
};

struct SceNetSockaddrIn {
    u8 len;
    SceNetSaFamily family;
    SceNetInPort port;
    SceNetInAddr addr;
    SceNetInPort vport;
    char pad[6];
};

s32* PS4_FUNC sceNetErrnoLoc();
u16 PS4_FUNC sceNetHtons(u16 host16);
u32 PS4_FUNC sceNetHtonl(u32 host32);
u32 PS4_FUNC sceNetNtohl(u32 net32);
u16 PS4_FUNC sceNetNtohs(u16 net16);
s32 PS4_FUNC sceNetInetPton(s32 af, const char* src, void* dst);
const char* PS4_FUNC sceNetInetNtop(int af, const void* src, char* dst, u32 size);
SceNetId PS4_FUNC sceNetSocket(const char* name, s32 family, s32 type, s32 protocol);
s32 PS4_FUNC sceNetBind(SceNetId s, const SceNetSockaddr* addr, SceNetSocklen addr_len);
s32 PS4_FUNC sceNetSendto(SceNetId s, const void* buf, size_t len, s32 flags, const SceNetSockaddr* addr, SceNetSocklen addr_len);
s32 PS4_FUNC sceNetRecvfrom(SceNetId s, void* buf, size_t len, s32 flags, SceNetSockaddr* addr, SceNetSocklen* addr_len);

}   // End namespace PS4::OS::Libs::SceNet