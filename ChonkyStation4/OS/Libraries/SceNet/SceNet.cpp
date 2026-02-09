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
    
    module.addSymbolStub("Nlev7Lg8k3A", "sceNetInit", "libSceNet", "libSceNet", 0);
    module.addSymbolStub("dgJBaeJnGpo", "sceNetPoolCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("C4UgDHHPvdw", "sceNetResolverCreate", "libSceNet", "libSceNet", 1);
    module.addSymbolStub("Nd91WaWmG2w", "sceNetResolverStartNtoa", "libSceNet", "libSceNet");
    module.addSymbolStub("kJlYH5uMAWI", "sceNetResolverDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("K7RlrTkI-mw", "sceNetPoolDestroy", "libSceNet", "libSceNet");
    module.addSymbolStub("OXXX4mUk3uk", "sceNetConnect", "libSceNet", "libSceNet", 0x80410100u | 35u);
    module.addSymbolStub("Q4qBuN-c0ZM", "sceNetSocket", "libSceNet", "libSceNet");
    module.addSymbolStub("2mKX2Spso7I", "sceNetSetsockopt", "libSceNet", "libSceNet");
    module.addSymbolStub("bErx49PgxyY", "sceNetBind", "libSceNet", "libSceNet");
    module.addSymbolStub("kOj1HiAGE54", "sceNetListen", "libSceNet", "libSceNet");
    module.addSymbolStub("cTGkc6-TBlI", "sceNetTerm", "libSceNet", "libSceNet");
}

static s32 sce_net_errno = 35;

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

}   // End namespace PS4::OS::Libs::SceNet