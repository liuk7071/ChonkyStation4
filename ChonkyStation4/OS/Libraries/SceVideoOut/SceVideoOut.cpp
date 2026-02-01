#include "SceVideoOut.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <GCN/GCN.hpp>


namespace PS4::OS::Libs::SceVideoOut {

MAKE_LOG_FUNCTION(log, lib_sceVideoOut);

void init(Module& module) {
    module.addSymbolExport("Up36PTk687E", "sceVideoOutOpen", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutOpen);
    module.addSymbolExport("uquVH4-Du78", "sceVideoOutClose", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutClose);
    module.addSymbolExport("OcQybQejHEY", "sceVideoOutGetBufferLabelAddress", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetBufferLabelAddress);
    module.addSymbolExport("CBiu4mCE1DA", "sceVideoOutSetFlipRate", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSetFlipRate);
    module.addSymbolExport("HXzjK9yI30k", "sceVideoOutAddFlipEvent", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutAddFlipEvent);
    module.addSymbolExport("w3BY+tAEiQY", "sceVideoOutRegisterBuffers", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutRegisterBuffers);
    module.addSymbolExport("i6-sR91Wt-4", "sceVideoOutSetBufferAttribute", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSetBufferAttribute);
    module.addSymbolExport("IOdgHlCGU-k", "sceVideoOutSubmitChangeBufferAttribute", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSubmitChangeBufferAttribute);
    module.addSymbolExport("U46NwOiJpys", "sceVideoOutSubmitFlip", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutSubmitFlip);
    module.addSymbolExport("SbU3dwp80lQ", "sceVideoOutGetFlipStatus", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetFlipStatus);
    module.addSymbolExport("1FZBKy8HeNU", "sceVideoOutGetVblankStatus", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetVblankStatus);
    module.addSymbolExport("6kPnj51T62Y", "sceVideoOutGetResolutionStatus", "libSceVideoOut", "libSceVideoOut", (void*)&sceVideoOutGetResolutionStatus);
    
    module.addSymbolStub("DYhhWbJSeRg", "sceVideoOutColorSettingsSetGamma_", "libSceVideoOut", "libSceVideoOut");
    module.addSymbolStub("pv9CI5VC+R0", "sceVideoOutAdjustColor_", "libSceVideoOut", "libSceVideoOut");
}

void SceVideoOutPort::signalFlip(u64 flip_arg) {
    flip_ev_source.trigger(flip_arg);
    flip_status.count++;
    //vblank_status.count++;
    //vblank_status.process_time = SDL_GetTicks64();
    //vblank_status.tsc = SDL_GetTicks64();
}

s32 PS4_FUNC sceVideoOutOpen(s32 uid, s32 bus_type, s32 idx, const void* param) {
    log("sceVideoOutOpen(uid=%d, bus_type=%d, idx=%d, param=%p)\n", uid, bus_type, idx, param);
    
    // Looks like Sonic Mania uses 0 as uid. I had to remove this check
    //if (uid != SCE_USER_SERVICE_USER_ID_SYSTEM) {
    //    Helpers::panic("sceVideoOutOpen: uid is not SCE_USER_SERVICE_USER_ID_SYSTEM\n");
    //}

    if (bus_type != SCE_VIDEO_OUT_BUS_TYPE_MAIN) {
        Helpers::panic("sceVideoOutOpen: bus_type is not SCE_VIDEO_OUT_BUS_TYPE_MAIN\n");
    }

    auto port = PS4::OS::make<SceVideoOutPort>();

    // Initialize event source
    port->flip_ev_source.init(SCE_VIDEO_OUT_FLIP_EVENT_ID, 0);  // TODO: Properly set filter

    std::memset(&port->flip_status, 0, sizeof(SceVideoOutFlipStatus));
    std::memset(&port->vblank_status, 0, sizeof(SceVideoOutVblankStatus));

    port->resolution_status.full_width = 1920;
    port->resolution_status.full_height = 1080;
    port->resolution_status.pane_width = 1920;
    port->resolution_status.pane_height = 1080;
    port->resolution_status.refresh_rate = 3; // SCE_VIDEO_OUT_REFRESH_RATE_59_94HZ
    port->resolution_status.screen_size_in_inch = 24.0f;
    port->resolution_status.flags = 0; // ?

    return port->handle;
}

s32 PS4_FUNC sceVideoOutClose(s32 handle) {
    log("sceVideoOutClose(handle=%d)\n", handle);

    if (!PS4::OS::erase(handle))
        Helpers::panic("sceVideoOutClose: handle %d does not exist\n", handle);
    return SCE_OK;
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
    port->flip_ev_source.addToEventQueue(eq, udata);
    return SCE_OK;
}

int reg_idx = 1;
s32 PS4_FUNC sceVideoOutRegisterBuffers(s32 handle, s32 start_idx, void** addrs, s32 n_bufs, SceVideoOutBufferAttribute* attrib) {
    log("sceVideoOutRegisterBuffers(handle=%d, start_idx=%d, addrs=*%p, n_bufs=%d, attrib=*%p)\n", handle, start_idx, addrs, n_bufs, attrib);

    for (int idx = start_idx; idx < start_idx + n_bufs; idx++) {
        bufs[idx].base = addrs[idx];
        bufs[idx].attrib = *attrib;
    }
    return reg_idx++;   // "Registration index" - return a progressive number for now
}

s32 PS4_FUNC sceVideoOutSetBufferAttribute(SceVideoOutBufferAttribute* attrib, u32 pixel_format, u32 tiling_mode, u32 aspect_ratio, u32 width, u32 height, u32 pitch_in_pixels) {
    log("sceVideoOutSetBufferAttribute(attrib=*%p, pixel_format=%d, tiling_mode=%d, aspect_ratio=%d, width=%d, height=%d, pitch_in_pixels=%d)\n", attrib, pixel_format, tiling_mode, aspect_ratio, width, height, pitch_in_pixels);

    attrib->pixel_format = pixel_format;
    attrib->tiling_mode = tiling_mode;
    attrib->aspect_ratio = aspect_ratio;
    attrib->width = width;
    attrib->height = height;
    attrib->pitch_in_pixels = pitch_in_pixels;
    attrib->option = 0; // SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_OPTION_NONE
    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutSubmitChangeBufferAttribute(s32 handle, s32 idx, SceVideoOutBufferAttribute* attrib) {
    log("sceVideoOutSubmitChangeBufferAttribute(handle=%d, idx=%d, attrib=*%p)\n", handle, idx, attrib);

    bufs[idx].attrib = *attrib;
    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutSubmitFlip(s32 handle, s32 buf_idx, s32 flip_mode, s64 flip_arg) {
    log("sceVideoOutSubmitFlip(handle=%d, buf_idx=%d, flip_mode=%d, flip_arg=0x%016llx)\n", handle, buf_idx, flip_mode, flip_arg);
    GCN::submitFlip(handle, buf_idx, flip_arg);
    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutGetFlipStatus(s32 handle, SceVideoOutFlipStatus* status) {
    log("sceVideoOutGetFlipStatus(handle=%d, status=*%p)\n", handle, status);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutGetFlipStatus: handle %d does not exist\n", handle);
    }

    *status = port->flip_status;
    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutGetVblankStatus(s32 handle, SceVideoOutVblankStatus* status) {
    log("sceVideoOutGetVblankStatus(handle=%d, status=*%p)\n", handle, status);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutGetFlipStatus: handle %d does not exist\n", handle);
    }

    // TODO: Stubbed to just increment every time it's polled
    port->vblank_status.count++;
    port->vblank_status.process_time = SDL_GetTicks64();
    port->vblank_status.tsc = SDL_GetTicks64();

    *status = port->vblank_status;
    return SCE_OK;
}

s32 PS4_FUNC sceVideoOutGetResolutionStatus(s32 handle, SceVideoOutResolutionStatus* status) {
    log("sceVideoOutGetResolutionStatus(handle=%d, status=*%p)\n", handle, status);

    auto port = PS4::OS::find<SceVideoOutPort>(handle);
    if (!port) {
        Helpers::panic("sceVideoOutGetResolutionStatus: handle %d does not exist\n", handle);
    }

    *status = port->resolution_status;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceVideoOut