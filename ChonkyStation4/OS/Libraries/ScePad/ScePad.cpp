#include "ScePad.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::ScePad {

MAKE_LOG_FUNCTION(log, lib_scePad);

void init(Module& module) {
    module.addSymbolExport("hv1luiJrqQM", "scePadInit", "libScePad", "libScePad", (void*)&scePadInit);
    
    module.addSymbolStub("xk0AcarP3V4", "scePadOpen", "libScePad", "libScePad", 1);
    module.addSymbolStub("6ncge5+l5Qs", "scePadClose", "libScePad", "libScePad");
    module.addSymbolStub("YndgXqQVV7c", "scePadReadState", "libScePad", "libScePad");
    module.addSymbolStub("DscD1i9HX1w", "scePadResetLightBar", "libScePad", "libScePad");
    module.addSymbolStub("rIZnR6eSpvk", "scePadResetOrientation", "libScePad", "libScePad");
}

s32 PS4_FUNC scePadInit() {
    log("scePadInit()\n");
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::ScePad