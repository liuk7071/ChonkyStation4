#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::ScePad {

void init(Module& module);

static constexpr s32 SCE_PAD_MAX_TOUCH_NUM = 2;
static constexpr s32 SCE_PAD_MAX_DEVICE_UNIQUE_DATA_SIZE = 12;

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
    u8 reserve1;
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

void clearButtons();
void pressButton(ScePadButtonDataOffset button);
void pollPads();

s32 PS4_FUNC scePadInit();
s32 PS4_FUNC scePadReadState(s32 handle, ScePadData* data);

}   // End namespace PS4::OS::Libs::ScePad