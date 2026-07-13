#include "SceNet.hpp"
#define ASIO_STANDALONE
#include <asio.hpp>
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <Common/ErrorCodes.hpp>
#include <OS/SceObj.hpp>
#include <OS/Libraries/Kernel/Equeue.hpp>
#ifdef _WIN32
#include <winsock.h>
#endif


namespace PS4::OS::Libs::SceNet {

MAKE_LOG_FUNCTION(log, lib_sceNet);

void init(Module& module) {
    module.addSymbolExport("HQOwnfMGipQ", "sceNetErrnoLoc", "libSceNet", "libSceNet", (void*)&sceNetErrnoLoc);
    module.addSymbolExport("SF47kB2MNTo", "sceNetEpollCreate", "libSceNet", "libSceNet", (void*)&sceNetEpollCreate);
    module.addSymbolExport("ZVw46bsasAk", "sceNetEpollControl", "libSceNet", "libSceNet", (void*)&sceNetEpollControl);
    module.addSymbolExport("drjIbDbA7UQ", "sceNetEpollWait", "libSceNet", "libSceNet", (void*)&sceNetEpollWait);
    module.addSymbolExport("J5i3hiLJMPk", "sceNetResolverGetError", "libSceNet", "libSceNet", (void*)&sceNetResolverGetError);
    module.addSymbolExport("iWQWrwiSt8A", "sceNetHtons", "libSceNet", "libSceNet", (void*)&sceNetHtons);
    module.addSymbolExport("9T2pDF2Ryqg", "sceNetHtonl", "libSceNet", "libSceNet", (void*)&sceNetHtonl);
    module.addSymbolExport("pQGpHYopAIY", "sceNetNtohl", "libSceNet", "libSceNet", (void*)&sceNetNtohl);
    module.addSymbolExport("Rbvt+5Y2iEw", "sceNetNtohs", "libSceNet", "libSceNet", (void*)&sceNetNtohs);
    module.addSymbolExport("8Kcp5d-q1Uo", "sceNetInetPton", "libSceNet", "libSceNet", (void*)&sceNetInetPton);
    module.addSymbolExport("Xn2TA2QhxHc", "sceNetInetPtonEx", "libSceNet", "libSceNet", (void*)&sceNetInetPtonEx);
    module.addSymbolExport("9vA2aW+CHuA", "sceNetInetNtop", "libSceNet", "libSceNet", (void*)&sceNetInetNtop);
    module.addSymbolExport("Q4qBuN-c0ZM", "sceNetSocket", "libSceNet", "libSceNet", (void*)&sceNetSocket);
    module.addSymbolExport("bErx49PgxyY", "sceNetBind", "libSceNet", "libSceNet", (void*)&sceNetBind);
    module.addSymbolExport("OXXX4mUk3uk", "sceNetConnect", "libSceNet", "libSceNet", (void*)&sceNetConnect);
    module.addSymbolExport("gvD1greCu0A", "sceNetSendto", "libSceNet", "libSceNet", (void*)&sceNetSendto);
    module.addSymbolExport("304ooNZxWDY", "sceNetRecvfrom", "libSceNet", "libSceNet", (void*)&sceNetRecvfrom);
    module.addSymbolExport("beRjXBn-z+o", "sceNetSend", "libSceNet", "libSceNet", (void*)&sceNetSend);
    module.addSymbolExport("9wO9XrMsNhc", "sceNetRecv", "libSceNet", "libSceNet", (void*)&sceNetRecv);
    module.addSymbolExport("xphrZusl78E", "sceNetGetsockopt", "libSceNet", "libSceNet", (void*)&sceNetGetsockopt);
    module.addSymbolExport("C4UgDHHPvdw", "sceNetResolverCreate", "libSceNet", "libSceNet", (void*)&sceNetResolverCreate);
    module.addSymbolExport("Nd91WaWmG2w", "sceNetResolverStartNtoa", "libSceNet", "libSceNet", (void*)&sceNetResolverStartNtoa);
    
    module.addSymbolStub("Nlev7Lg8k3A", "sceNetInit", "libSceNet", "libSceNet", 0);
    module.addSymbolStub("dgJBaeJnGpo", "sceNetPoolCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("kJlYH5uMAWI", "sceNetResolverDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("K7RlrTkI-mw", "sceNetPoolDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("PIWqhn9oSxc", "sceNetAccept", "libSceNet", "libSceNet");
    module.addSymbolStub("2mKX2Spso7I", "sceNetSetsockopt", "libSceNet", "libSceNet");
    module.addSymbolStub("kOj1HiAGE54", "sceNetListen", "libSceNet", "libSceNet");
    module.addSymbolStub("9wO9XrMsNhc", "sceNetRecv", "libSceNet", "libSceNet");
    module.addSymbolStub("hoOAofhhRvE", "sceNetGetsockname", "libSceNet", "libSceNet");
    module.addSymbolStub("Apb4YDxKsRI", "sceNetResolverStartAton", "libSceNet", "libSceNet");
    module.addSymbolStub("TSM6whtekok", "sceNetShutdown", "libSceNet", "libSceNet");
    module.addSymbolStub("zJGf8xjFnQE", "sceNetSocketAbort", "libSceNet", "libSceNet");
    module.addSymbolStub("45ggEzakPJQ", "sceNetSocketClose", "libSceNet", "libSceNet");
    module.addSymbolStub("6Oc0bLsIYe0", "sceNetGetMacAddress", "libSceNet", "libSceNet");
    module.addSymbolStub("Inp1lfL+Jdw", "sceNetEpollDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("cTGkc6-TBlI", "sceNetTerm", "libSceNet", "libSceNet");

    // libSceNetCtl
    module.addSymbolExport("uBPlr0lbuiI", "sceNetCtlGetState", "libSceNetCtl", "libSceNetCtl", (void*)&sceNetCtlGetState);
    module.addSymbolExport("obuxdTiwkF8", "sceNetCtlGetInfo", "libSceNetCtl", "libSceNetCtl", (void*)&sceNetCtlGetInfo);
    
    module.addSymbolStub("gky0+oaNM4k", "sceNetCtlInit", "libSceNetCtl", "libSceNetCtl");
    module.addSymbolStub("UJ+Z7Q+4ck0", "sceNetCtlRegisterCallback", "libSceNetCtl", "libSceNetCtl");   // Should store callback id
    module.addSymbolStub("Rqm2OnZMCz0", "sceNetCtlUnregisterCallback", "libSceNetCtl", "libSceNetCtl");
    module.addSymbolStub("iQw3iQPhvUQ", "sceNetCtlCheckCallback", "libSceNetCtl", "libSceNetCtl");   // Should store callback id
    module.addSymbolStub("JO4yuTuMoKI", "sceNetCtlGetNatInfo", "libSceNetCtl", "libSceNetCtl");
    module.addSymbolStub("Z4wwCFiBELQ", "sceNetCtlTerm", "libSceNetCtl", "libSceNetCtl");
}

static s32 sce_net_errno = 0;

using asio::ip::tcp;
using asio::ip::udp;

struct SceSocket : SceObj {
    s32 type = 0;

    asio::io_context io;
    std::unique_ptr<tcp::socket> tcp_sock;
    std::unique_ptr<udp::socket> udp_sock;
};

struct SceNetEpoll : SceObj {
    Kernel::Equeue equeue;
};

struct SceNetResolver : SceObj {
    Kernel::EventSource ev_source;
    s32 error = 0;
};

std::pair<std::string, int> sockAddrInToIpPortPair(const SceNetSockaddrIn* addr) {
    char str[64];
    if (sceNetInetNtop(SCE_NET_AF_INET, &addr->addr, str, sizeof(str)) == nullptr) {
        Helpers::panic("sockAddrInToIpPortPair: sceNetInetNtop returned error\n");
    }

    return { std::string(str), sceNetNtohs(addr->port) };
}

s32* PS4_FUNC sceNetErrnoLoc() {
    return &sce_net_errno;
}

SceNetId PS4_FUNC sceNetEpollCreate(const char* name, int flags) {
    log("sceNetEpollCreate(name=\"%s\", flags=%d)\n", name, flags);

    auto* epoll = OS::make<SceNetEpoll>();
    return epoll->handle;
}

s32 PS4_FUNC sceNetEpollControl(SceNetId eid, s32 op, SceNetId id, SceNetEpollEvent* event) {
    log("sceNetEpollControl(eid=%d, op=%d, id=%d, event=*%p)\n", eid, op, id, event);
    
    auto* epoll = OS::find<SceNetEpoll>(eid);
    if (!epoll) {
        Helpers::panic("sceNetEpollControl: epoll %d does not exist\n", eid);
    }

    // For now we assume that the id is a DNS resolver, because socket epolls are not implemented
    auto* resolver = OS::find<SceNetResolver>(id);
    if (!resolver) {
        Helpers::panic("sceNetEpollControl: socket or resolver %d does not exist\n", id);
    }

    switch (op) {
    case SCE_NET_EPOLL_CTL_ADD: {
        // Register the epoll's event queue to the socket/resolver event source
        resolver->ev_source.addToEventQueue(&epoll->equeue, event->data.ptr);
        break;
    }
    default:    printf("sceNetEpoll: unhandled op %d\n", op);
    }

    return SCE_OK;
}

s32 PS4_FUNC sceNetEpollWait(SceNetId eid, SceNetEpollEvent* events, s32 max_events, s32 timeout) {
    log("sceNetEpollWait(eid=%d, events=*%p, max_events=%d, timeout=%d)\n", eid, events, max_events, timeout);

    auto* epoll = OS::find<SceNetEpoll>(eid);
    if (!epoll) {
        Helpers::panic("sceNetEpollWait: epoll %d does not exist\n", eid);
    }

    // Wait for events
    auto [timed_out, recv_events] = epoll->equeue.wait(timeout);
    if (timed_out) {
        return 0;
    }

    if (recv_events.size() > max_events) {
        Helpers::panic("TODO: sceNetEpollWait recv_events.size() > max_events\n");
    }

    // For each event:
    // - ident contains the socket/resolver ID
    // - data contains the event (i.e. SCE_NET_EPOLLDESCID)
    // - udata contains the user data (SceNetEpollEvent::data)
    const auto n_events = std::min<size_t>(recv_events.size(), max_events);
    for (int i = 0; i < recv_events.size(); i++) {
        events[i].events    = recv_events[i].data;
        events[i].reserved  = 0;
        events[i].ident     = recv_events[i].ident;
        events[i].data.ptr  = recv_events[i].udata;
    }
    return n_events;
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

s32 PS4_FUNC sceNetInetPtonEx(s32 af, const char* src, void* dst, int flags) {
    log("sceNetInetPtonEx(af=%d, src=\"%s\", dst=*%p, flags=%d) [forwarding to sceNetInetPtonEx]\n", af, src, dst, flags);
    return sceNetInetPton(af, src, dst);
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

    auto* sock = OS::make<SceSocket>();
    sock->type = type;
    
    switch (sock->type) {
    case SCE_NET_SOCK_STREAM: {
        sock->tcp_sock = std::make_unique<tcp::socket>(sock->io);
        break;
    }
    case SCE_NET_SOCK_DGRAM: {
        sock->udp_sock = std::make_unique<udp::socket>(sock->io);
        break;
    }
    default: {
        log("warning: unsupported socket type %d (TODO)\n", type);
    }
    }

    return sock->handle;
}

s32 PS4_FUNC sceNetBind(SceNetId s, const SceNetSockaddr* addr, SceNetSocklen addr_len) {
    log("sceNetBind(s=%d, addr=*%p, addr_len=%d)\n", s, addr, addr_len);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetBind: socket does not exist\n");
    }

    try {
        switch (sock->type) {
        case SCE_NET_SOCK_STREAM: {
            auto* bind_addr = (SceNetSockaddrIn*)addr;
            const auto port = sceNetNtohs(bind_addr->port);
            log("binding tcp socket to port %d (TODO)\n", port);
            //sock->tcp_sock->bind(tcp::endpoint(tcp::v4(), port));
            break;
        }
        case SCE_NET_SOCK_DGRAM: {
            auto* bind_addr = (SceNetSockaddrIn*)addr;
            const auto port = sceNetNtohs(bind_addr->port);
            log("binding udp socket to port %d\n", port);
            sock->udp_sock->bind(udp::endpoint(udp::v4(), port));
            break;
        }
        }
    }
    catch (const asio::system_error& e) {
        Helpers::panic(e.what());
    }
    
    return 0;
}

s32 PS4_FUNC sceNetConnect(SceNetId s, const SceNetSockaddr* addr, SceNetSocklen addr_len) {
    log("sceNetConnect(s=%d, addr=*%p, addr_len=%d)\n", s, addr, addr_len);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetConnect: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_STREAM: {
        auto* conn_addr = (SceNetSockaddrIn*)addr;
        sock->tcp_sock = std::make_unique<tcp::socket>(sock->io);

        auto [ip, port] = sockAddrInToIpPortPair(conn_addr);
        log("connecting tcp socket to %s:%d\n", ip.c_str(), port);
        sock->tcp_sock->connect(tcp::endpoint(asio::ip::make_address_v4(ip), port));
        break;
    }
    case SCE_NET_SOCK_DGRAM: {
        Helpers::panic("TODO: sceNetConnect on UDP socket\n");
        break;
    }
    }

    return SCE_OK;
}

s32 PS4_FUNC sceNetSendto(SceNetId s, const void* buf, size_t len, s32 flags, const SceNetSockaddr* addr, SceNetSocklen addr_len) {
    log("sceNetSendto(s=%d, buf=%p, len=%lld, flags=%d, addr=*%p, addr_len=%d)\n", s, buf, len, flags, addr, addr_len);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetSendto: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_STREAM: {
        Helpers::panic("TODO: sceNetSendto on TCP socket\n");
        break;
    }
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
        Helpers::panic("sceNetRecvfrom: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_STREAM: {
        Helpers::panic("TODO: sceNetRecvfrom on TCP socket\n");
        break;
    }
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

s32 PS4_FUNC sceNetSend(SceNetId s, const void* buf, size_t len, int flags) {
    log("sceNetSend(s=%d, buf=%p, len=%lld, flags=%d)\n", s, buf, len, flags);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetSend: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_STREAM: {
        return sock->tcp_sock->send(asio::buffer(buf, len));
    }
    case SCE_NET_SOCK_DGRAM: {
        Helpers::panic("TODO: sceNetSend on UDP socket\n");
        break;
    }
    }

    return SCE_OK;
}

s32 PS4_FUNC sceNetRecv(SceNetId s, void* buf, size_t len, int flags) {
    log("sceNetRecv(s=%d, buf=%p, len=%lld, flags=%d)\n", s, buf, len, flags);

    auto* sock = OS::find<SceSocket>(s);
    if (!sock) {
        Helpers::panic("sceNetRecv: socket does not exist\n");
    }

    switch (sock->type) {
    case SCE_NET_SOCK_STREAM: {
        return sock->tcp_sock->read_some(asio::buffer(buf, len));
    }
    case SCE_NET_SOCK_DGRAM: {
        Helpers::panic("TODO: sceNetRecv on UDP socket\n");
        break;
    }
    }

    return SCE_OK;
}

s32 PS4_FUNC sceNetGetsockopt(SceNetId s, s32 level, s32 option_name, void* val, SceNetSocklen* option_len) {
    log("sceNetGetsockopt(s=%d, level=%d, option_name=%d, val=%p, option_len=*%p)\n", s, level, option_name, val, option_len);

    return SCE_OK;
}

SceNetId PS4_FUNC sceNetResolverCreate(const char* name, s32 memid, s32 flags) {
    log("sceNetResolverCreate(name=\"%s\", memid=%d, flags=%d)\n", name, memid, flags);

    auto* resolver = OS::make<SceNetResolver>();
    resolver->ev_source.init(resolver->handle, 0);
    return resolver->handle;
}

SceNetId /* doesn't actually return an id */ PS4_FUNC sceNetResolverStartNtoa(SceNetId resolver_id, const char* hostname, SceNetInAddr* addr, int timeout, int retry, int flags) {
    log("sceNetResolverStartNtoa(resolver_id=%d, hostname=\"%s\", addr=*%p, timeout=%d, retry=%d, flags=%d)\n", resolver_id, hostname, addr, timeout, retry, flags);

    auto* resolver = OS::find<SceNetResolver>(resolver_id);
    if (!resolver) {
        Helpers::panic("sceNetResolverStartNtoa: resolver %d does not exist\n", resolver_id);
    }

    addrinfo* res;
    int err = getaddrinfo(hostname, nullptr, nullptr, &res);
    if (err) {
        log("failed to resolve hostname \"%s\"\n", hostname);

        // Return error only if it's not an async operation
        if (!(flags & 1)) {
            *sceNetErrnoLoc() = 64; // SCE_NET_EHOSTDOWN
            return SCE_KERNEL_ERROR_EHOSTDOWN;  // Or HOSTUNREACH? Or ENOHOST?
        }
        else {
            resolver->error = SCE_NET_ERROR_RESOLVER_ENOHOST;
            return SCE_OK;
        }
    }

    *addr = *(SceNetInAddr*)&(((sockaddr_in*)res->ai_addr)->sin_addr);

    // Print address, also useful to check if the stored address is valid
    char str[64];
    if (sceNetInetNtop(SCE_NET_AF_INET, addr, str, sizeof(str)) == nullptr) {
        Helpers::panic("sceNetResolverStartNtoa: sceNetInetNtop returned error\n");
    }
    freeaddrinfo(res);

    // Send event
    resolver->ev_source.trigger(SCE_NET_EPOLLDESCID);

    log("resolved hostname \"%s\" as %s\n", hostname, str);
    resolver->error = SCE_OK;
    return SCE_OK;
}

SceNetId /* doesn't actually return an id */ PS4_FUNC sceNetResolverGetError(SceNetId resolver_id, s32* result) {
    log("sceNetResolverGetError(resolver_id=%d, result=*%p)\n", resolver_id, result);

    auto* resolver = OS::find<SceNetResolver>(resolver_id);
    if (!resolver) {
        Helpers::panic("sceNetResolverGetError: resolver %d does not exist\n", resolver_id);
    }

    *result = resolver->error;
    return SCE_OK;
}

// SceNetCtl (TODO: Move to separate file)

s32 PS4_FUNC sceNetCtlGetState(s32* state) {
    log("sceNetCtlGetState()\n");
    //*state = 0; // Disconnected
    *state = 3; // IP Obtained
    return SCE_OK;
}

s32 PS4_FUNC sceNetCtlGetInfo(s32 code, SceNetCtlInfo* info) {
    log("sceNetCtlGetInfo(code=%d, info=*%p)\n", code, info);

    switch (code) {
    case SCE_NET_CTL_INFO_HTTP_PROXY_CONFIG: {
        info->http_proxy_config = 0;
        break;
    }

    case SCE_NET_CTL_INFO_HTTP_PROXY_SERVER: {
        std::memset(info->http_proxy_server, 0, SCE_NET_CTL_HOSTNAME_LEN);
        break;
    }

    case SCE_NET_CTL_INFO_HTTP_PROXY_PORT: {
        info->http_proxy_port = 0;
        break;
    }

    default: {
        printf("sceNetCtlGetInfo: unhandled info code %d\n", code);
    }
    }

    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNet