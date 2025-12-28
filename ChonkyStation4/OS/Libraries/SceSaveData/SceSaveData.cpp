#include "SceSaveData.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSaveData {

MAKE_LOG_FUNCTION(log, lib_sceSaveData);

void init(Module& module) {
    module.addSymbolStub("ZkZhskCPXFw", "sceSaveDataInitialize", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("TywrFKCoLGY", "sceSaveDataInitialize3", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("0z45PIH+SNI", "sceSaveDataMount2", "libSceSaveData", "libSceSaveData", 0x809f000a /* no space */);
    module.addSymbolStub("v7AAAMo0Lz4", "sceSaveDataSetupSaveDataMemory", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("7Bt5pBC-Aco", "sceSaveDataGetSaveDataMemory", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("h3YURzXGSVQ", "sceSaveDataSetSaveDataMemory", "libSceSaveData", "libSceSaveData");
    module.addSymbolStub("dyIhnXq-0SM", "sceSaveDataDirNameSearch", "libSceSaveData", "libSceSaveData");
}

}   // End namespace PS4::OS::Libs::SceSysmodule