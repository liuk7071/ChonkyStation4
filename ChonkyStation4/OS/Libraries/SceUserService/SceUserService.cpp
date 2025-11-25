#include "SceUserService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceUserService {

MAKE_LOG_FUNCTION(log, lib_sceUserService);

void init(Module& module) {
    module.addSymbolStub("j3YMu1MVNNo", "sceUserServiceInitialize", "libSceUserService", "libSceUserService");
}

}   // End namespace PS4::OS::Libs::SceNpManager