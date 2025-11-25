#include "SceSysmodule.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSysmodule {

MAKE_LOG_FUNCTION(log, lib_sceUserService);

void init(Module& module) {
    module.addSymbolStub("g8cM39EUZ6o", "sceSysmoduleLoadModule", "libSceSysmodule", "libSceSysmodule");
}

}   // End namespace PS4::OS::Libs::SceSysmodule