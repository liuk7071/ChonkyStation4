#include "ScePad.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>


namespace PS4::OS::Libs::ScePad {

MAKE_LOG_FUNCTION(log, lib_scePad);

void init(Module& module) {
    module.addSymbolExport("hv1luiJrqQM", "scePadInit", "libScePad", "libScePad", (void*)&scePadInit);
    module.addSymbolExport("xk0AcarP3V4", "scePadOpen", "libScePad", "libScePad", (void*)&scePadOpen);
    module.addSymbolExport("YndgXqQVV7c", "scePadReadState", "libScePad", "libScePad", (void*)&scePadReadState);
    module.addSymbolExport("q1cHNfGycLI", "scePadRead", "libScePad", "libScePad", (void*)&scePadRead);
    module.addSymbolExport("gjP9-KQzoUk", "scePadGetControllerInformation", "libScePad", "libScePad", (void*)&scePadGetControllerInformation);
    module.addSymbolExport("RR4novUEENY", "scePadSetLightBar", "libScePad", "libScePad", (void*)&scePadSetLightBar);
    module.addSymbolExport("yFVnOdGxvZY", "scePadSetVibration", "libScePad", "libScePad", (void*)&scePadSetVibration);
    
    module.addSymbolStub("6ncge5+l5Qs", "scePadClose", "libScePad", "libScePad");
    module.addSymbolStub("DscD1i9HX1w", "scePadResetLightBar", "libScePad", "libScePad");
    module.addSymbolStub("rIZnR6eSpvk", "scePadResetOrientation", "libScePad", "libScePad");
    module.addSymbolStub("clVvL4ZDntw", "scePadSetMotionSensorState", "libScePad", "libScePad");
}

ScePadData pad_state;

SDL_GameController* findController() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return nullptr;
}

void clearState() {
    pad_state.buttons = 0;
    pad_state.left_stick.x = 0x80;
    pad_state.left_stick.y = 0x80;
    pad_state.right_stick.x = 0x80;
    pad_state.right_stick.y = 0x80;
    pad_state.analog_buttons.l2 = 0;
    pad_state.analog_buttons.r2 = 0;
}

void pressButton(ScePadButtonDataOffset button) {
    pad_state.buttons |= button;
}

void setStickX(ScePadAnalogStick& stick, float val) {
    stick.x = std::clamp<float>(((val + 1.0f) / 2.0f) * 255, 0, 255);
}

void setStickY(ScePadAnalogStick& stick, float val) {
    stick.y = std::clamp<float>(((val + 1.0f) / 2.0f) * 255, 0, 255);
}

static constexpr float deadzone = 0.10f;

void setStickXWithDeadzone(ScePadAnalogStick& stick, float val) {
    // Deadzone
    if (val < -deadzone || val > deadzone)
        setStickX(stick, val);
}

void setStickYWithDeadzone(ScePadAnalogStick& stick, float val) {
    // Deadzone
    if (val < -deadzone || val > deadzone)
        setStickY(stick, val);
}

void pollPads() {
    auto* keys = SDL_GetKeyboardState(nullptr);

    clearState();

    if (!controller) {
        if (keys[SDL_SCANCODE_LEFT])    pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT);
        if (keys[SDL_SCANCODE_RIGHT])   pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT);
        if (keys[SDL_SCANCODE_UP])      pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_UP);
        if (keys[SDL_SCANCODE_DOWN])    pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN);
        if (keys[SDL_SCANCODE_SPACE])   pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_CROSS);
        if (keys[SDL_SCANCODE_Z])       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_SQUARE);
        if (keys[SDL_SCANCODE_X])       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_CIRCLE);
        if (keys[SDL_SCANCODE_C])       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_TRIANGLE);
        if (keys[SDL_SCANCODE_A])       setStickX(pad_state.left_stick, -1.0f);
        if (keys[SDL_SCANCODE_D])       setStickX(pad_state.left_stick, 1.0f);;
        if (keys[SDL_SCANCODE_W])       setStickY(pad_state.left_stick, -1.0f);
        if (keys[SDL_SCANCODE_S])       setStickY(pad_state.left_stick, 1.0f);
        if (keys[SDL_SCANCODE_J])       setStickX(pad_state.right_stick, -1.0f);
        if (keys[SDL_SCANCODE_L])       setStickX(pad_state.right_stick, 1.0f);;
        if (keys[SDL_SCANCODE_I])       setStickY(pad_state.right_stick, -1.0f);
        if (keys[SDL_SCANCODE_K])       setStickY(pad_state.right_stick, 1.0f);
    }
    else {
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START))           pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_OPTIONS);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))      pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))         pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_UP);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))       pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A))               pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_CROSS);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B))               pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_CIRCLE);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X))               pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_SQUARE);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y))               pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_TRIANGLE);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))    pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_L1);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))   pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_R1);
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_TOUCHPAD))        pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_TOUCH_PAD);
        float left_x  = (float)SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX)  / 32767.0f;
        float left_y  = (float)SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY)  / 32767.0f;
        float right_x = (float)SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
        float right_y = (float)SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
        setStickXWithDeadzone(pad_state.left_stick,  left_x);
        setStickYWithDeadzone(pad_state.left_stick,  left_y);
        setStickXWithDeadzone(pad_state.right_stick, right_x);
        setStickYWithDeadzone(pad_state.right_stick, right_y);
        pad_state.analog_buttons.l2 = ((float)SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT)  / 32767.0f) * 255;
        pad_state.analog_buttons.r2 = ((float)SDL_GameControllerGetAxis(controller, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f) * 255;
        if (pad_state.analog_buttons.l2 > 220) pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_L2);
        if (pad_state.analog_buttons.r2 > 220) pressButton(ScePadButtonDataOffset::SCE_PAD_BUTTON_R2);
    }

    pad_state.timestamp = SDL_GetTicks64();
}

s32 PS4_FUNC scePadInit() {
    log("scePadInit()\n");
    
    controller = findController();
    std::memset(&pad_state, 0, sizeof(ScePadData));
    pad_state.connected = true;
    pad_state.connected_count = 1;
    pad_state.left_stick.x = 0x80;
    pad_state.left_stick.y = 0x80;
    pad_state.right_stick.x = 0x80;
    pad_state.right_stick.y = 0x80;
    return SCE_OK;
}

bool opened = false;

s32 PS4_FUNC scePadOpen(s32 uid, s32 type, s32 idx, const ScePadOpenParam* param) {
    log("scePadOpen(uid=%d, type=%d, idx=%d, param=*%p)\n", uid, type, idx, param);

    if (uid == Libs::SceUserService::SCE_USER_SERVICE_USER_ID_INVALID)
        return Libs::SceUserService::SCE_DEVICE_SERVICE_ERROR_INVALID_USER;

    if (opened) return SCE_PAD_ERROR_ALREADY_OPENED;
    opened = true;

    return 1;   // Handle
}

s32 PS4_FUNC scePadReadState(s32 handle, ScePadData* data) {
    log("scePadReadState(handle=%d, data=*%p)\n", handle, data);
    if (handle != 1) return SCE_PAD_ERROR_INVALID_HANDLE;

    *data = pad_state;
    return SCE_OK;
}

s32 PS4_FUNC scePadRead(s32 handle, ScePadData* data, s32 num) {
    log("scePadRead(handle=%d, data=*%p, num=%d)\n", handle, data, num);
    //if (handle != 1) return SCE_PAD_ERROR_INVALID_HANDLE;

    // TODO: We just return the current state
    *data = pad_state;
    return 1;
}

s32 PS4_FUNC scePadGetControllerInformation(s32 handle, ScePadControllerInformation* info) {
    log("scePadGetControllerInformation(handle=%d, info=*%p)\n", handle, info);

    info->touchpad_info.pixel_density = 1;
    info->touchpad_info.resolution.x = 1920;
    info->touchpad_info.resolution.y = 950;
    info->stick_info.deadzone_left = 1;
    info->stick_info.deadzone_right = 1;
    info->connection_type = SCE_PAD_CONNECTION_TYPE_LOCAL;
    info->connected_count = 1;
    info->connected = true;
    info->device_class = ScePadDeviceClass::SCE_PAD_DEVICE_CLASS_STANDARD;
    return SCE_OK;
}

s32 PS4_FUNC scePadSetLightBar(s32 handle, const ScePadLightBarParam* param) {
    log("scePadSetLightBar(handle=%d, param=*%p)\n", handle, param);

    if (controller)
        SDL_GameControllerSetLED(controller, param->r, param->g, param->b);
    return SCE_OK;
}

s32 PS4_FUNC scePadSetVibration(s32 handle, const ScePadVibrationParam* param) {
    log("scePadSetVibration(handle=%d, param=*%p)\n", handle, param);

    if (controller)
        SDL_GameControllerRumble(controller, (param->large_motor << 8) | param->large_motor, (param->small_motor << 8) | param->small_motor, -1);
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::ScePad