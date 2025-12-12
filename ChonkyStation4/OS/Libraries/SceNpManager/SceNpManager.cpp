#include "SceNpManager.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceNpManager {

MAKE_LOG_FUNCTION(log, lib_sceNpManager);

void init(Module& module) {
    module.addSymbolStub("Ec63y59l9tw", "sceNpSetNpTitleId", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("A2CQ3kgSopQ", "sceNpSetContentRestriction", "libSceNpManager", "libSceNpManager");
    module.addSymbolStub("p-o74CnoNzY", "sceNpGetNpId", "libSceNpManager", "libSceNpManager", 0x8055000a /* user not signed up */);
}

}   // End namespace PS4::OS::Libs::SceNpManager