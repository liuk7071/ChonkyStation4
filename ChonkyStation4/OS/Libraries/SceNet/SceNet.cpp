#include "SceNet.hpp"
#define ASIO_STANDALONE
#include <asio.hpp>
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#ifdef _WIN32
#include <winsock.h>
#endif


namespace PS4::OS::Libs::SceNet {

MAKE_LOG_FUNCTION(log, lib_sceNet);

void init(Module& module) {
    module.addSymbolExport("HQOwnfMGipQ", "sceNetErrnoLoc", "libSceNet", "libSceNet", (void*)&sceNetErrnoLoc);
    module.addSymbolExport("iWQWrwiSt8A", "sceNetHtons", "libSceNet", "libSceNet", (void*)&sceNetHtons);
    module.addSymbolExport("9T2pDF2Ryqg", "sceNetHtonl", "libSceNet", "libSceNet", (void*)&sceNetHtonl);
    module.addSymbolExport("pQGpHYopAIY", "sceNetNtohl", "libSceNet", "libSceNet", (void*)&sceNetNtohl);
    module.addSymbolExport("Rbvt+5Y2iEw", "sceNetNtohs", "libSceNet", "libSceNet", (void*)&sceNetNtohs);
    module.addSymbolExport("8Kcp5d-q1Uo", "sceNetInetPton", "libSceNet", "libSceNet", (void*)&sceNetInetPton);
    module.addSymbolExport("9vA2aW+CHuA", "sceNetInetNtop", "libSceNet", "libSceNet", (void*)&sceNetInetNtop);
    module.addSymbolExport("Q4qBuN-c0ZM", "sceNetSocket", "libSceNet", "libSceNet", (void*)&sceNetSocket);
    module.addSymbolExport("bErx49PgxyY", "sceNetBind", "libSceNet", "libSceNet", (void*)&sceNetBind);
    module.addSymbolExport("gvD1greCu0A", "sceNetSendto", "libSceNet", "libSceNet", (void*)&sceNetSendto);
    module.addSymbolExport("304ooNZxWDY", "sceNetRecvfrom", "libSceNet", "libSceNet", (void*)&sceNetRecvfrom);
    
    module.addSymbolStub("Nlev7Lg8k3A", "sceNetInit", "libSceNet", "libSceNet", 0);
    module.addSymbolStub("dgJBaeJnGpo", "sceNetPoolCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("C4UgDHHPvdw", "sceNetResolverCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("Nd91WaWmG2w", "sceNetResolverStartNtoa", "libSceNet", "libSceNet");
    module.addSymbolStub("kJlYH5uMAWI", "sceNetResolverDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("K7RlrTkI-mw", "sceNetPoolDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("OXXX4mUk3uk", "sceNetConnect", "libSceNet", "libSceNet", 0x80410100u | 35u);
    module.addSymbolStub("PIWqhn9oSxc", "sceNetAccept", "libSceNet", "libSceNet");
    module.addSymbolStub("drjIbDbA7UQ", "sceNetEpollWait", "libSceNet", "libSceNet");
    module.addSymbolStub("2mKX2Spso7I", "sceNetSetsockopt", "libSceNet", "libSceNet");
    module.addSymbolStub("xphrZusl78E", "sceNetGetsockopt", "libSceNet", "libSceNet");
    module.addSymbolStub("kOj1HiAGE54", "sceNetListen", "libSceNet", "libSceNet");
    module.addSymbolStub("9wO9XrMsNhc", "sceNetRecv", "libSceNet", "libSceNet");
    module.addSymbolStub("hoOAofhhRvE", "sceNetGetsockname", "libSceNet", "libSceNet");
    module.addSymbolStub("Apb4YDxKsRI", "sceNetResolverStartAton", "libSceNet", "libSceNet");
    module.addSymbolStub("SF47kB2MNTo", "sceNetEpollCreate", "libSceNet", "libSceNet");
    module.addSymbolStub("ZVw46bsasAk", "sceNetEpollControl", "libSceNet", "libSceNet");
    module.addSymbolStub("TSM6whtekok", "sceNetShutdown", "libSceNet", "libSceNet");
    module.addSymbolStub("zJGf8xjFnQE", "sceNetSocketAbort", "libSceNet", "libSceNet");
    module.addSymbolStub("45ggEzakPJQ", "sceNetSocketClose", "libSceNet", "libSceNet");
    module.addSymbolStub("6Oc0bLsIYe0", "sceNetGetMacAddress", "libSceNet", "libSceNet");
    module.addSymbolStub("Inp1lfL+Jdw", "sceNetEpollDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("cTGkc6-TBlI", "sceNetTerm", "libSceNet", "libSceNet");
}

static s32 sce_net_errno = 0;

using asio::ip::tcp;
using asio::ip::udp;

struct SceSocket : SceObj {
    s32 type = 0;

    asio::io_context io;
    std::unique_ptr<udp::socket> udp_sock;
};

s32* PS4_FUNC sceNetErrnoLoc() {
    return &sce_net_errno;
}

u16 PS4_FUNC sceNetHtons(u16 host16) {
    log("sceNetHtons(host16=0x%x)\n", host16);
    return htons(host16);
}

u32 PS4_FUNC sceNetHtonl(u32 host32) {
    log("sceNetHtonl(host32=0x%x)\n", host32);
    return htonl(host32);
}

u32 PS4_FUNC sceNetNtohl(u32 net32) {
    log("sceNetNtohl(net32=0x%x)\n", net32);
    return ntohl(net32);
}

u16 PS4_FUNC sceNetNtohs(u16 net16) {
    log("sceNetNtohs(net16=0x%x)\n", net16);
    return ntohs(net16);
}

s32 PS4_FUNC sceNetInetPton(s32 af, const char* src, void* dst) {
    log("sceNetInetPton(af=%d, src=\"%s\", dst=*%p)\n", af, src, dst);
    return inet_pton(af, src, dst);
}

const char* PS4_FUNC sceNetInetNtop(int af, const void* src, char* dst, u32 size) {
    log("sceNetInetNtop(af=%d, src=%p, dst=*%p, size=%d)\n", af, src, dst, size);
    return inet_ntop(af, src, dst, size);
}

SceNetId PS4_FUNC sceNetSocket(const char* name, s32 family, s32 type, s32 protocol) {
    log("sceNetSocket(name=\"%s\", family=%d, type=%d, protocol=%d)\n", name, family, type, protocol);

    if (family != SCE_NET_AF_INET) {
        Helpers::panic("sceNetSocket: family is not SCE_NET_AF_INET\n");
    }

    // For now only UDP sockets are implemented
    if (type != SCE_NET_SOCK_DGRAM) {
        log("warning: unsupported type %d (TODO)\n", type);
    }

    auto* sock = OS::make<SceSocket>();
    sock->type = type;
    return sock->handle;
}

s32 PS4_FUNC sceNetBind(SceNetId s, const SceNetSockaddr* addr, SceNetSocklen addr_len) {
    log("sceNetBind(s=%d, addr=*%p, addr_len=%d)\n", s, addr, addr_len);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetBind: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_DGRAM: {
        auto* bind_addr = (SceNetSockaddrIn*)addr;
        log("binding socket to port %d\n", bind_addr->port);

        sock->udp_sock = std::make_unique<udp::socket>(sock->io, udp::endpoint(udp::v4(), bind_addr->port));
        break;
    }
    }
    
    return 0;
}

s32 PS4_FUNC sceNetSendto(SceNetId s, const void* buf, size_t len, s32 flags, const SceNetSockaddr* addr, SceNetSocklen addr_len) {
    log("sceNetSendto(s=%d, buf=%p, len=%lld, flags=%d, addr=*%p, addr_len=%d)\n", s, buf, len, flags, addr, addr_len);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetBind: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_DGRAM: {
        auto* send_addr = (SceNetSockaddrIn*)addr;

        // Convert addr back to a string
        char str[64];
        if (sceNetInetNtop(SCE_NET_AF_INET, &send_addr->addr, str, sizeof(str)) == nullptr) {
            Helpers::panic("sceNetSendto: sceNetInetNtop returned error\n");
        }
        const std::string ip = std::string(str);
        const auto port = sceNetNtohs(send_addr->port);
        log("sending %d bytes to %s:%d\n", len, ip.c_str(), port);

        udp::endpoint endpoint(asio::ip::make_address(ip), port);
        return sock->udp_sock->send_to(asio::buffer(buf, len), endpoint);
    }
    }

    return 0;
}

s32 PS4_FUNC sceNetRecvfrom(SceNetId s, void* buf, size_t len, s32 flags, SceNetSockaddr* addr, SceNetSocklen* addr_len) {
    log("sceNetRecvfrom(s=%d, buf=%p, len=%lld, flags=%d, addr=*%p, addr_len=*%p)\n", s, buf, len, flags, addr, addr_len);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetBind: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_DGRAM: {
        auto* in_addr = (SceNetSockaddrIn*)addr;
        udp::endpoint endpoint;
        const auto size_received = sock->udp_sock->receive_from(asio::buffer(buf, len), endpoint);
        
        in_addr->len = sizeof(*in_addr);
        in_addr->family = SCE_NET_AF_INET;
        if (sceNetInetPton(SCE_NET_AF_INET, endpoint.address().to_string().c_str(), &in_addr->addr) <= 0) {
            Helpers::panic("sceNetRecvfrom: sceNetInetPton returned error\n");
        }
        in_addr->port = sceNetHtons(endpoint.port());

        return size_received;
    }
    }

    return 0;
}

}   // End namespace PS4::OS::Libs::SceNet