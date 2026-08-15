#include "SceSysmodule.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSysmodule {

MAKE_LOG_FUNCTION(log, lib_sceSysmodule);

void init(Module& module) {
    module.addSymbolExport("CU8m+Qs+HN4", "sceSysmoduleLoadModuleByNameInternal", "libSceSysmodule", "libSceSysmodule", (void*)&sceSysmoduleLoadModuleByNameInternal);
    
    module.addSymbolStub("g8cM39EUZ6o", "sceSysmoduleLoadModule", "libSceSysmodule", "libSceSysmodule", 0);
    module.addSymbolStub("39iV5E1HoCk", "sceSysmoduleLoadModuleInternal", "libSceSysmodule", "libSceSysmodule", SCE_OK);
    module.addSymbolStub("fMP5NHUOaMk", "sceSysmoduleIsLoaded", "libSceSysmodule", "libSceSysmodule", SCE_OK /* module is loaded */);
    module.addSymbolStub("ynFKQ5bfGks", "sceSysmoduleIsLoadedInternal", "libSceSysmodule", "libSceSysmodule", SCE_OK /* module is loaded */);
    module.addSymbolStub("eR2bZFAAU0Q", "sceSysmoduleUnloadModule", "libSceSysmodule", "libSceSysmodule");
    module.addSymbolStub("hHrGoGoNf+s", "sceSysmoduleLoadModuleInternalWithArg", "libSceSysmodule", "libSceSysmodule");
    module.addSymbolStub("D8cuU4d72xM", "sceSysmoduleGetModuleHandleInternal", "libSceSysmodule", "libSceSysmodule", 0);
    module.addSymbolStub("vXZhrtJxkGc", "sceSysmoduleUnloadModuleInternal", "libSceSysmodule", "libSceSysmodule", 0);
}

s32 PS4_FUNC sceSysmoduleLoadModuleByNameInternal(char* name) {
    log("sceSysmoduleLoadModuleByNameInternal(name=\"%s\")\n", name);

    return 0;
}

}   // End namespace PS4::OS::Libs::SceSysmodule