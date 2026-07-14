#include "SceAudio3d.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceAudio3d {

MAKE_LOG_FUNCTION(log, lib_sceAudio3d);

void init(Module& module) {
    module.addSymbolExport("VEVhZ9qd4ZY", "sceAudio3dPortPush", "libSceAudio3d", "libSceAudio3d", (void*)&sceAudio3dPortPush);
    module.addSymbolExport("lw0qrdSjZt8", "sceAudio3dPortAdvance", "libSceAudio3d", "libSceAudio3d", (void*)&sceAudio3dPortAdvance);
    module.addSymbolExport("YaaDbDwKpFM", "sceAudio3dPortGetQueueLevel", "libSceAudio3d", "libSceAudio3d", (void*)&sceAudio3dPortGetQueueLevel);
    
    module.addSymbolStub("UmCvjSmuZIw", "sceAudio3dInitialize", "libSceAudio3d", "libSceAudio3d");
    module.addSymbolStub("XeDDK0xJWQA", "sceAudio3dPortOpen", "libSceAudio3d", "libSceAudio3d");
    module.addSymbolStub("jO2tec4dJ2M", "sceAudio3dObjectReserve", "libSceAudio3d", "libSceAudio3d");
    module.addSymbolStub("4uyHN9q4ZeU", "sceAudio3dObjectSetAttributes", "libSceAudio3d", "libSceAudio3d");
    module.addSymbolStub("Yq9bfUQ0uJg", "sceAudio3dPortSetAttribute", "libSceAudio3d", "libSceAudio3d");
}

s32 PS4_FUNC sceAudio3dPortPush(SceAudio3dPortId port_id, SceAudio3dBlocking blocking) {
    log("sceAudio3dPortPush(port_id=%d, blocking=%d)\n", port_id, blocking);

    // TODO: Check for the blocking mode (sync or async)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return SCE_OK;
}

s32 PS4_FUNC sceAudio3dPortAdvance(SceAudio3dPortId port_id) {
    log("sceAudio3dPortAdvance(port_id=%d)\n", port_id);
    return SCE_AUDIO3D_ERROR_NOT_READY;
}

s32 PS4_FUNC sceAudio3dPortGetQueueLevel(SceAudio3dPortId port_id, u32* pending, u32* available) {
    log("sceAudio3dPortGetQueueLevel(port_id=%d, pending=*%p, available=*%p)\n", port_id, pending, available);

    if (pending)
        *pending = 0;
    if (available)
        *available = 4;

    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceAudio3d