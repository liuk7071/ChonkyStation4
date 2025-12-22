#include "SceNpTrophy.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceNpTrophy {

MAKE_LOG_FUNCTION(log, lib_sceNpTrophy);

void init(Module& module) {
    module.addSymbolStub("XbkjbobZlCY", "sceNpTrophyCreateContext", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("q7U6tEAQf7c", "sceNpTrophyCreateHandle", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("TJCAxto9SEU", "sceNpTrophyRegisterContext", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("LHuSmO3SLd8", "sceNpTrophyGetTrophyUnlockState", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("28xmRUFao68", "sceNpTrophyUnlockTrophy", "libSceNpTrophy", "libSceNpTrophy");
    module.addSymbolStub("GNcF4oidY0Y", "sceNpTrophyDestroyHandle", "libSceNpTrophy", "libSceNpTrophy");
}

}   // End namespace PS4::OS::Libs::SceSysmodule