#include "ScePad.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <SDL.h>


namespace PS4::OS::Libs::ScePad {

MAKE_LOG_FUNCTION(log, lib_scePad);

void init(Module& module) {
    module.addSymbolExport("hv1luiJrqQM", "scePadInit", "libScePad", "libScePad", (void*)&scePadInit);
    module.addSymbolExport("YndgXqQVV7c", "scePadReadState", "libScePad", "libScePad", (void*)&scePadReadState);
    
    module.addSymbolStub("xk0AcarP3V4", "scePadOpen", "libScePad", "libScePad", 1);
    module.addSymbolStub("6ncge5+l5Qs", "scePadClose", "libScePad", "libScePad");
    module.addSymbolStub("DscD1i9HX1w", "scePadResetLightBar", "libScePad", "libScePad");
    module.addSymbolStub("rIZnR6eSpvk", "scePadResetOrientation", "libScePad", "libScePad");
}

ScePadData pad_state;

void clearButtons() {
    pad_state.buttons = 0;
}

void pressButton(ScePadButtonDataOffset button) {
    pad_state.buttons |= button;
}

void pollPads() {
    auto* keys = SDL_GetKeyboardState(nullptr);

    clearButtons();
    if (keys[SDL_SCANCODE_LEFT])    pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT);
    if (keys[SDL_SCANCODE_RIGHT])   pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT);
    if (keys[SDL_SCANCODE_UP])      pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_UP);
    if (keys[SDL_SCANCODE_DOWN])    pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN);
    if (keys[SDL_SCANCODE_SPACE])   pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_CROSS);
    if (keys[SDL_SCANCODE_Z])       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_SQUARE);
    if (keys[SDL_SCANCODE_X])       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_CIRCLE);
    if (keys[SDL_SCANCODE_C])       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_TRIANGLE);

    pad_state.timestamp = SDL_GetTicks64();
}

s32 PS4_FUNC scePadInit() {
    log("scePadInit()\n");

    std::memset(&pad_state, 0, sizeof(ScePadData));
    pad_state.connected = true;
    pad_state.connected_count = 1;
    pad_state.left_stick.x = 0x80;
    pad_state.left_stick.y = 0x80;
    pad_state.right_stick.x = 0x80;
    pad_state.right_stick.y = 0x80;
    return SCE_OK;
}

s32 PS4_FUNC scePadReadState(s32 handle, ScePadData* data) {
    *data = pad_state;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::ScePad