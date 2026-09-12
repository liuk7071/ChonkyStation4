#include "SceIme.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <atomic>


namespace PS4::OS::Libs::SceIme {

MAKE_LOG_FUNCTION(log, lib_sceIme);

void init(Module& module) {
    module.addSymbolExport("RPydv-Jr1bc", "sceImeOpen", "libSceIme", "libSceIme", (void*)&sceImeOpen);
    module.addSymbolExport("-4GCfYdNF1s", "sceImeUpdate", "libSceIme", "libSceIme", (void*)&sceImeUpdate);
    module.addSymbolExport("TmVP8LzcFcY", "sceImeClose", "libSceIme", "libSceIme", (void*)&sceImeClose);
    
    module.addSymbolStub("uTW+63goeJs", "InitializeImeModule", "libSceIme", "libSceIme");
    module.addSymbolStub("eaFXjfJv3xs", "sceImeKeyboardOpen", "libSceIme", "libSceIme");
    module.addSymbolStub("WmYDzdC4EHI", "sceImeParamInit", "libSceIme", "libSceIme");
    module.addSymbolStub("VkqLPArfFdc", "sceImeKeyboardGetInfo", "libSceIme", "libSceIme");
    module.addSymbolStub("dKadqZFgKKQ", "sceImeKeyboardGetResourceId", "libSceIme", "libSceIme");
    module.addSymbolStub("ieCNrVrzKd4", "sceImeSetText", "libSceIme", "libSceIme");
    module.addSymbolStub("WLxUN2WMim8", "sceImeSetCaret", "libSceIme", "libSceIme");
    module.addSymbolStub("TXYHFRuL8UY", "sceImeSetTextGeometry", "libSceIme", "libSceIme");
}

std::atomic<bool> opened;
SceImeParam ime_param;

s32 PS4_FUNC sceImeOpen(const SceImeParam* param, const SceImeParamExtended* extended_param) {
    log("sceImeOpen(param=*%p, extended_param=*%p)\n", param, extended_param);

    if (opened)
        return SCE_IME_ERROR_BUSY;

    opened = true;
    ime_param = *param;
    return SCE_OK;
}

s32 PS4_FUNC sceImeUpdate(SceImeEventHandler handler) {
    log("sceImeUpdate(handler=%p)\n", handler);

    if (!opened)
        return SCE_IME_ERROR_NOT_OPENED;

    SceImeEvent event;
    
    event.id = SceImeEventId::SCE_IME_EVENT_OPEN;
    handler(ime_param.arg, &event);
    
    wchar_t* text = L"Chonky";
    event.id = SceImeEventId::SCE_IME_EVENT_UPDATE_TEXT;
    event.param.text.str = text;
    event.param.text.caret_idx = 6;
    event.param.text.n_areas = 1;
    std::memset(event.param.text.text_areas, 0, sizeof(event.param.text.text_areas));
    event.param.text.text_areas[0].mode   = SceImeTextAreaMode::SCE_IME_TEXT_AREA_MODE_EDIT;
    event.param.text.text_areas[0].index  = 0;
    event.param.text.text_areas[0].length = 6;

    handler(ime_param.arg, &event);

    event.id = SceImeEventId::SCE_IME_EVENT_PRESS_ENTER;
    handler(ime_param.arg, &event);
    return SCE_OK;
}

s32 PS4_FUNC sceImeClose() {
    log("sceImeClose()\n");

    if (!opened)
        return SCE_IME_ERROR_NOT_OPENED;

    opened = false;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceIme