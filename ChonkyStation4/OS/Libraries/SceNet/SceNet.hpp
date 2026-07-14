#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceNet {

void init(Module& module);

static constexpr s32 SCE_NET_ERROR_RESOLVER_ENOHOST = 0x804101e6;

static constexpr s32 SCE_NET_AF_INET = 2;
static constexpr s32 SCE_NET_SOCK_STREAM = 1;
static constexpr s32 SCE_NET_SOCK_DGRAM = 2;
static constexpr s32 SCE_NET_SOCK_RAW = 3;
static constexpr s32 SCE_NET_SOCK_DGRAM_P2P = 6;
static constexpr s32 SCE_NET_SOCK_STREAM_P2P = 0;

static constexpr s32 SCE_NET_EPOLL_CTL_ADD = 1;
static constexpr s32 SCE_NET_EPOLL_CTL_MOD = 2;
static constexpr s32 SCE_NET_EPOLL_CTL_DEL = 3;

static constexpr s32 SCE_NET_EPOLLIN        = 0x1;
static constexpr s32 SCE_NET_EPOLLOUT       = 0x2;
static constexpr s32 SCE_NET_EPOLLERR       = 0x8;
static constexpr s32 SCE_NET_EPOLLHUP       = 0x10;
static constexpr s32 SCE_NET_EPOLLDESCID    = 0x10000;

using SceNetId = s32;
using SceNetSaFamily = u8;
using SceNetInPort = u16;
using SceNetInAddr = u32;
using SceNetSocklen = u32;
using SceNetInAddr = u32;

union SceNetEpollData {
    void* ptr;
    u32 u32;
    int fd;
    u64 u64;
};

struct SceNetEpollEvent {
    u32 events;
    u32 reserved;
    u64 ident;
    SceNetEpollData data;
};

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
SceNetId PS4_FUNC sceNetEpollCreate(const char* name, int flags);
s32 PS4_FUNC sceNetEpollControl(SceNetId eid, s32 op, SceNetId id, SceNetEpollEvent* event);
s32 PS4_FUNC sceNetEpollWait(SceNetId eid, SceNetEpollEvent* events, s32 max_events, s32 timeout);
u16 PS4_FUNC sceNetHtons(u16 host16);
u32 PS4_FUNC sceNetHtonl(u32 host32);
u32 PS4_FUNC sceNetNtohl(u32 net32);
u16 PS4_FUNC sceNetNtohs(u16 net16);
s32 PS4_FUNC sceNetInetPton(s32 af, const char* src, void* dst);
s32 PS4_FUNC sceNetInetPtonEx(s32 af, const char* src, void* dst, int flags);
const char* PS4_FUNC sceNetInetNtop(int af, const void* src, char* dst, u32 size);
SceNetId PS4_FUNC sceNetSocket(const char* name, s32 family, s32 type, s32 protocol);
s32 PS4_FUNC sceNetBind(SceNetId s, const SceNetSockaddr* addr, SceNetSocklen addr_len);
s32 PS4_FUNC sceNetConnect(SceNetId s, const SceNetSockaddr* addr, SceNetSocklen addr_len);
s32 PS4_FUNC sceNetSendto(SceNetId s, const void* buf, size_t len, s32 flags, const SceNetSockaddr* addr, SceNetSocklen addr_len);
s32 PS4_FUNC sceNetRecvfrom(SceNetId s, void* buf, size_t len, s32 flags, SceNetSockaddr* addr, SceNetSocklen* addr_len);
s32 PS4_FUNC sceNetSend(SceNetId s, const void* buf, size_t len, int flags);
s32 PS4_FUNC sceNetRecv(SceNetId s, void* buf, size_t len, int flags);
s32 PS4_FUNC sceNetGetsockopt(SceNetId s, s32 level, s32 option_name, void* val, SceNetSocklen* option_len);
s32 PS4_FUNC kernel_socket(s32 family, s32 type, s32 protocol);
SceNetId PS4_FUNC sceNetResolverCreate(const char* name, s32 memid, s32 flags);
SceNetId PS4_FUNC sceNetResolverStartNtoa(SceNetId resolver_id, const char* hostname, SceNetInAddr* addr, int timeout, int retry, int flags);
SceNetId PS4_FUNC sceNetResolverGetError(SceNetId resolver_id, s32* result);

// SceNetCtl

static constexpr s32 SCE_NET_CTL_SSID_LEN           = 32 + 1;
static constexpr s32 SCE_NET_CTL_HOSTNAME_LEN       = 255 + 1;
static constexpr s32 SCE_NET_CTL_AUTH_NAME_LEN      = 127 + 1;
static constexpr s32 SCE_NET_CTL_IPV4_ADDR_STR_LEN  = 16;
static constexpr s32 SCE_NET_ETHER_ADDR_LEN         = 6;

static constexpr s32 SCE_NET_CTL_INFO_DEVICE            = 1;
static constexpr s32 SCE_NET_CTL_INFO_ETHER_ADDR        = 2;
static constexpr s32 SCE_NET_CTL_INFO_MTU               = 3;
static constexpr s32 SCE_NET_CTL_INFO_LINK              = 4;
static constexpr s32 SCE_NET_CTL_INFO_BSSID             = 5;
static constexpr s32 SCE_NET_CTL_INFO_SSID              = 6;
static constexpr s32 SCE_NET_CTL_INFO_WIFI_SECURITY     = 7;
static constexpr s32 SCE_NET_CTL_INFO_RSSI_DBM          = 8;
static constexpr s32 SCE_NET_CTL_INFO_RSSI_PERCENTAGE   = 9;
static constexpr s32 SCE_NET_CTL_INFO_CHANNEL           = 10;
static constexpr s32 SCE_NET_CTL_INFO_IP_CONFIG         = 11;
static constexpr s32 SCE_NET_CTL_INFO_DHCP_HOSTNAME     = 12;
static constexpr s32 SCE_NET_CTL_INFO_PPPOE_AUTH_NAME   = 13;
static constexpr s32 SCE_NET_CTL_INFO_IP_ADDRESS        = 14;
static constexpr s32 SCE_NET_CTL_INFO_NETMASK           = 15;
static constexpr s32 SCE_NET_CTL_INFO_DEFAULT_ROUTE     = 16;
static constexpr s32 SCE_NET_CTL_INFO_PRIMARY_DNS       = 17;
static constexpr s32 SCE_NET_CTL_INFO_SECONDARY_DNS     = 18;
static constexpr s32 SCE_NET_CTL_INFO_HTTP_PROXY_CONFIG = 19;
static constexpr s32 SCE_NET_CTL_INFO_HTTP_PROXY_SERVER = 20;
static constexpr s32 SCE_NET_CTL_INFO_HTTP_PROXY_PORT   = 21;

struct SceNetEtherAddr {
    u8 data[SCE_NET_ETHER_ADDR_LEN];
};

union SceNetCtlInfo {
    u32 device;
    SceNetEtherAddr ether_addr;
    u32 mtu;
    u32 link;
    SceNetEtherAddr bssid;
    char ssid[SCE_NET_CTL_SSID_LEN];
    u32 wifi_security;
    u8 rssi_dbm;
    u8 rssi_percentage;
    u8 channel;
    u32 ip_config;
    char dhcp_hostname[SCE_NET_CTL_HOSTNAME_LEN];
    char pppoe_auth_name[SCE_NET_CTL_AUTH_NAME_LEN];
    char ip_address[SCE_NET_CTL_IPV4_ADDR_STR_LEN];
    char netmask[SCE_NET_CTL_IPV4_ADDR_STR_LEN];
    char default_route[SCE_NET_CTL_IPV4_ADDR_STR_LEN];
    char primary_dns[SCE_NET_CTL_IPV4_ADDR_STR_LEN];
    char secondary_dns[SCE_NET_CTL_IPV4_ADDR_STR_LEN];
    u32 http_proxy_config;
    char http_proxy_server[SCE_NET_CTL_HOSTNAME_LEN];
    u16 http_proxy_port;
};

s32 PS4_FUNC sceNetCtlGetState(s32* state);
s32 PS4_FUNC sceNetCtlGetInfo(s32 code, SceNetCtlInfo* info);

}   // End namespace PS4::OS::Libs::SceNet