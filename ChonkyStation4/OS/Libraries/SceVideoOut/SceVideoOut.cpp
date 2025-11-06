#include "SceVideoOut.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceVideoOut {

MAKE_LOG_FUNCTION(log, lib_sceVideoOut);

void init(Module& module) {
    module.addSymbolExport("Up36PTk687E", "sceVideoOutOpen", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutOpen);
    module.addSymbolExport("OcQybQejHEY", "sceVideoOutGetBufferLabelAddress", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetBufferLabelAddress);
    module.addSymbolExport("CBiu4mCE1DA", "sceVideoOutSetFlipRate", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSetFlipRate);
}

s32 PS4_FUNC sceVideoOutOpen(s32 uid, s32 bus_type, s32 idx, const void* param) {
    log("sceVideoOutOpen(uid=%d, bus_type=%d, idx=%d, param=%p)\n", uid, bus_type, idx, param);
    
    if (uid != SCE_USER_SERVICE_USER_ID_SYSTEM) {
        Helpers::panic("sceVideoOutOpen: uid is not SCE_USER_SERVICE_USER_ID_SYSTEM\n");
    }

    if (bus_type != SCE_VIDEO_OUT_BUS_TYPE_MAIN) {
        Helpers::panic("sceVideoOutOpen: bus_type is not SCE_VIDEO_OUT_BUS_TYPE_MAIN\n");
    }

    auto port = PS4::OS::make<SceVideoOutPort>();
    return port->handle;
}

s32 PS4_FUNC sceVideoOutGetBufferLabelAddress(s32 handle, void** label_addr) {
    log("sceVideoOutGetBufferLabelAddress(handle=%d, label_addr=*%p)\n", handle, label_addr);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutGetBufferLabelAddress: handle %d does not exist\n", handle);
    }

    *label_addr = port->buffer_labels;
    return 16;  // Number of display buffers?
}

s32 PS4_FUNC sceVideoOutSetFlipRate(s32 handle, s32 rate) {
    log("sceVideoOutSetFlipRate(handle=%d, rate=%d) TODO\n", handle, rate);

    return SCE_OK;
}

}