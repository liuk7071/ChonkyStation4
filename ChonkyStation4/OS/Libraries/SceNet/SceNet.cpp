#include "SceNet.hpp"
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
    module.addSymbolExport("9vA2aW+CHuA", "sceNetInetNtop", "libSceNet", "libSceNet", (void*)&sceNetInetNtop);
    
    module.addSymbolStub("Nlev7Lg8k3A", "sceNetInit", "libSceNet", "libSceNet", 0);
    module.addSymbolStub("8Kcp5d-q1Uo", "sceNetInetPton", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("dgJBaeJnGpo", "sceNetPoolCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("C4UgDHHPvdw", "sceNetResolverCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("Nd91WaWmG2w", "sceNetResolverStartNtoa", "libSceNet", "libSceNet");
    module.addSymbolStub("kJlYH5uMAWI", "sceNetResolverDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("K7RlrTkI-mw", "sceNetPoolDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("OXXX4mUk3uk", "sceNetConnect", "libSceNet", "libSceNet", 0x80410100u | 35u);
    module.addSymbolStub("PIWqhn9oSxc", "sceNetAccept", "libSceNet", "libSceNet");
    module.addSymbolStub("Q4qBuN-c0ZM", "sceNetSocket", "libSceNet", "libSceNet");
    module.addSymbolStub("2mKX2Spso7I", "sceNetSetsockopt", "libSceNet", "libSceNet");
    module.addSymbolStub("xphrZusl78E", "sceNetGetsockopt", "libSceNet", "libSceNet");
    module.addSymbolStub("bErx49PgxyY", "sceNetBind", "libSceNet", "libSceNet");
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

const char* PS4_FUNC sceNetInetNtop(int af, const void* src, char* dst, u32 size) {
    log("sceNetInetNtop(af=%d, src=%p, dst=*%p, size=%d) TODO\n", af, src, dst, size);

    // TODO
    std::strncpy(dst, "127.0.0.1", size);
    return dst;
}

}   // End namespace PS4::OS::Libs::SceNet