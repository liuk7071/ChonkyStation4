#include "SceSystemService.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceSystemService {

MAKE_LOG_FUNCTION(log, lib_sceSystemService);

void init(Module& module) {
    module.addSymbolExport("fZo48un7LK4", "sceSystemServiceParamGetInt", "libSceSystemService", "libSceSystemService", (void*)&sceSystemServiceParamGetInt);
}

s32 PS4_FUNC sceSystemServiceParamGetInt(SceSystemServiceParamId param_id, s32* val) {
    log("sceSystemServiceParamGetInt(param_id=%d, value=*%p)\n", param_id, val);

    s32 ret = 0;
    switch (param_id) {
    case SCE_SYSTEM_SERVICE_PARAM_ID_LANG: ret = 1; break;  // English (United States)

    default:
        Helpers::panic("sceSystemServiceParamGetInt: unhandled param_id=%d\n", param_id);
    }

    *val = ret;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpManager