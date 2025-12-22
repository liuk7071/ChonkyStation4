#pragma once

#include <Common.hpp>
#include <SDL.h>


class Module;

namespace PS4::OS::Libs::ScePad {

void init(Module& module);

inline SDL_GameController* controller;

static constexpr s32 SCE_PAD_ERROR_INVALID_HANDLE = 0x80920003;
static constexpr s32 SCE_PAD_ERROR_ALREADY_OPENED = 0x80920004;

static constexpr s32 SCE_PAD_MAX_TOUCH_NUM = 2;
static constexpr s32 SCE_PAD_MAX_DEVICE_UNIQUE_DATA_SIZE = 12;
static constexpr s32 SCE_PAD_CONNECTION_TYPE_LOCAL = 0;

enum ScePadButtonDataOffset {
    SCE_PAD_BUTTON_L3           = 0x00000002,
    SCE_PAD_BUTTON_R3           = 0x00000004,
    SCE_PAD_BUTTON_OPTIONS      = 0x00000008,
    SCE_PAD_BUTTON_UP           = 0x00000010,
    SCE_PAD_BUTTON_RIGHT        = 0x00000020,
    SCE_PAD_BUTTON_DOWN         = 0x00000040,
    SCE_PAD_BUTTON_LEFT         = 0x00000080,
    SCE_PAD_BUTTON_L2           = 0x00000100,
    SCE_PAD_BUTTON_R2           = 0x00000200,
    SCE_PAD_BUTTON_L1           = 0x00000400,
    SCE_PAD_BUTTON_R1           = 0x00000800,
    SCE_PAD_BUTTON_TRIANGLE     = 0x00001000,
    SCE_PAD_BUTTON_CIRCLE       = 0x00002000,
    SCE_PAD_BUTTON_CROSS        = 0x00004000,
    SCE_PAD_BUTTON_SQUARE       = 0x00008000,
    SCE_PAD_BUTTON_TOUCH_PAD    = 0x00100000,
    SCE_PAD_BUTTON_INTERCEPTED  = 0x80000000,
};

struct ScePadAnalogStick {
    u8 x;
    u8 y;
};

struct ScePadAnalogButtons {
    u8 l2;
    u8 r2;
    u8 padding[2];
};

struct SceFQuaternion {
    float x, y, z, w;
};

struct SceFVector3 {
    float x, y, z;
};

struct ScePadTouch {
    u16 x;
    u16 y;
    u8 id;
    u8 reserve[3];
};

struct ScePadTouchData {
    u8 n_touch;
    u8 reserve[3];
    u32 reserve1;
    ScePadTouch touch[SCE_PAD_MAX_TOUCH_NUM];
};

struct ScePadExtensionUnitData {
    u32 extension_unit_id;
    u8 reserve[1];
    u8 data_len;
    u8 data[10];
};

struct ScePadData {
    u32 buttons;
    ScePadAnalogStick left_stick;
    ScePadAnalogStick right_stick;
    ScePadAnalogButtons analog_buttons;
    SceFQuaternion orientation;
    SceFVector3 acceleration;
    SceFVector3 angular_velocity;
    ScePadTouchData touch_data;
    bool connected;
    u64 timestamp;
    ScePadExtensionUnitData extension_unit_data;
    u8 connected_count;
    u8 reserve[2];
    u8 device_unique_data_len;
    u8 device_unique_data[SCE_PAD_MAX_DEVICE_UNIQUE_DATA_SIZE];
};

enum class ScePadDeviceClass {
    SCE_PAD_DEVICE_CLASS_INVALID = -1,
    SCE_PAD_DEVICE_CLASS_STANDARD = 0,
    SCE_PAD_DEVICE_CLASS_GUITAR = 1,
    SCE_PAD_DEVICE_CLASS_DRUM = 2,
    SCE_PAD_DEVICE_CLASS_DJ_TURNTABLE = 3,
    SCE_PAD_DEVICE_CLASS_DANCEMAT = 4,
    SCE_PAD_DEVICE_CLASS_NAVIGATION = 5,
    SCE_PAD_DEVICE_CLASS_STEERING_WHEEL = 6,
    SCE_PAD_DEVICE_CLASS_STICK = 7,
    SCE_PAD_DEVICE_CLASS_FLIGHT_STICK = 8,
    SCE_PAD_DEVICE_CLASS_GUN = 9,
};

struct ScePadTouchPadInformation {
    float pixel_density;
    struct {
        u16 x;
        u16 y;
    } resolution;
};

struct ScePadStickInformation {
    u8 deadzone_left;
    u8 deadzone_right;
};

struct ScePadControllerInformation {
    ScePadTouchPadInformation touchpad_info;
    ScePadStickInformation stick_info;
    u8 connection_type;
    u8 connected_count;
    bool connected;
    ScePadDeviceClass device_class;
    u8 reserve[8];
};

struct ScePadOpenParam {
    u8 reserved[8];
};

void clearButtons();
void pressButton(ScePadButtonDataOffset button);
void pollPads();

s32 PS4_FUNC scePadInit();
s32 PS4_FUNC scePadOpen(s32 uid, s32 type, s32 idx, const ScePadOpenParam* param);
s32 PS4_FUNC scePadReadState(s32 handle, ScePadData* data);
s32 PS4_FUNC scePadGetControllerInformation(s32 handle, ScePadControllerInformation* info);

}   // End namespace PS4::OS::Libs::ScePad