#include "SceSaveData.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSaveData {

MAKE_LOG_FUNCTION(log, lib_sceSaveData);

void init(Module& module) {
    module.addSymbolStub("TywrFKCoLGY", "sceSaveDataInitialize3", "libSceSaveData", "libSceSaveData");
}

}   // End namespace PS4::OS::Libs::SceSysmodule