#include "SceVideoOut.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceVideoOut {

MAKE_LOG_FUNCTION(log, lib_sceVideoOut);

void init(Module& module) {
    module.addSymbolExport("Up36PTk687E", "sceVideoOutOpen", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutOpen);
    module.addSymbolExport("OcQybQejHEY", "sceVideoOutGetBufferLabelAddress", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetBufferLabelAddress);
    module.addSymbolExport("CBiu4mCE1DA", "sceVideoOutSetFlipRate", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSetFlipRate);
    module.addSymbolExport("HXzjK9yI30k", "sceVideoOutAddFlipEvent", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutAddFlipEvent);
    module.addSymbolExport("w3BY+tAEiQY", "sceVideoOutRegisterBuffers", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutRegisterBuffers);
    module.addSymbolExport("IOdgHlCGU-k", "sceVideoOutSubmitChangeBufferAttribute", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSubmitChangeBufferAttribute);
    module.addSymbolExport("U46NwOiJpys", "sceVideoOutSubmitFlip", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSubmitFlip);
    module.addSymbolExport("SbU3dwp80lQ", "sceVideoOutGetFlipStatus", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetFlipStatus);
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

s32 PS4_FUNC sceVideoOutAddFlipEvent(Kernel::SceKernelEqueue eq, s32 handle, void* udata) {
    log("sceVideoOutAddFlipEvent(eq=*%p, handle=%d, udata=*%p) TODO\n", eq, handle, udata);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutAddFlipEvent: handle %d does not exist\n", handle);
    }

    // TODO: Check if the eq exists?
    eq->registerEvent({
        .ident = SCE_VIDEO_OUT_FLIP_EVENT_ID,
        .filter = 0,    // TODO
        .flags = 0,     // TODO
        .fflags = 0,    // TODO
        .data = 0,      // TODO
        .udata = udata,
    });
    port->flip_eqs.push_back(eq);
    return SCE_OK;
}

int reg_idx = 0;
s32 PS4_FUNC sceVideoOutRegisterBuffers(s32 handle, s32 start_idx, void** addrs, s32 n_bufs, SceVideoOutBufferAttribute* attrib) {
    log("sceVideoOutRegisterBuffers(handle=%d, start_idx=%d, addrs=*%p, n_bufs=%d, attrib=*%p) TODO\n", handle, start_idx, addrs, n_bufs, attrib);

    return reg_idx++;   // "Registration index" - return a progressive number for now
}

s32 PS4_FUNC sceVideoOutSubmitChangeBufferAttribute(s32 handle, s32 idx, SceVideoOutBufferAttribute* attrib) {
    log("sceVideoOutSubmitChangeBufferAttribute(handle=%d, idx=%d, attrib=*%p) TODO\n", handle, idx, attrib);

    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutSubmitFlip(s32 handle, s32 buf_idx, s32 flip_mode, s64 flip_arg) {
    log("sceVideoOutSubmitFlip(handle=%d, buf_idx=%d, flip_mode=%d, flip_arg=0x%016llx) TODO\n", handle, buf_idx, flip_mode, flip_arg);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutSubmitFlip: handle %d does not exist\n", handle);
    }

    // Stubbed for now
    for (auto& eq : port->flip_eqs) {
        eq->trigger(SCE_VIDEO_OUT_FLIP_EVENT_ID, 0, 0); // TODO: Fill in proper flip event data
    }

    port->flip_status.count++;
    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutGetFlipStatus(s32 handle, SceVideoOutFlipStatus* status) {
    log("sceVideoOutGetFlipStatus(handle=%d, status=%p)\n", handle, status);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutGetFlipStatus: handle %d does not exist\n", handle);
    }

    *status = port->flip_status;
    return SCE_OK;
}

}