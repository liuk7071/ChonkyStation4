#include "SceSaveData.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSaveData {

MAKE_LOG_FUNCTION(log, lib_sceSaveData);

void init(Module& module) {
    module.addSymbolStub("ZkZhskCPXFw", "sceSaveDataInitialize", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("TywrFKCoLGY", "sceSaveDataInitialize3", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("0z45PIH+SNI", "sceSaveDataMount2", "libSceSaveData", "libSceSaveData", 0x809f000a);
}

}   // End namespace PS4::OS::Libs::SceSysmodule