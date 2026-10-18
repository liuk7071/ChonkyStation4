// #pragma once
//
// #include <Common.hpp>
//
//
// class Module;
//
// namespace PS4::OS::Libs::SceMbus {
//
// void init(Module& module);
//
// struct SceMbusEvent {
//     u32 system;
//     u32 event_id;
//     u64 device_id;
//     u32 unk1;
//     u32 subsystem;
//     u64 unk2;
//     u64 unk3;
//     u64 unk4;
// };
//
// void submitEvent(const SceMbusEvent& event);
//
// struct SceMbusDeviceInfo;
//
// s32 PS4_FUNC sceMbusEventReceive(u32 handle, SceMbusEvent* out_events, u32 n_events, u32 unk1 /* maybe timeout? */);
// s32 PS4_FUNC sceMbusGetDeviceInfoByCondition_(u32* unk1, u64 unk2, void* unk3, u64 unk4, u64* out_n_devices, SceMbusDeviceInfo* out_info, u64 unk7);
//
// }   // End namespace PS4::OS::Libs::SceMbus

#pragma once

#include <Common.hpp>
#include <cstddef>


class Module;

namespace PS4::OS::Libs::SceMbus {

void init(Module& module);

struct SceMbusEvent {
    u32 type;
    u32 reserved;
    u64 device_id;
    u8 payload[64];
};
static_assert(sizeof(SceMbusEvent) == 80);

void submitEvent(const SceMbusEvent& event);

struct SceMbusDeviceInfo {
    u64 device_id;
    s32 transport;
    s32 type;
    u32 flags;
    s32 function_count;
    struct Function { s32 type, reserved, state, reserved2; } functions[11];
    s32 user_id;
    u8 reserved[52];
};
static_assert(sizeof(SceMbusDeviceInfo) == 256);
static_assert(offsetof(SceMbusDeviceInfo, user_id) == 200);

using EventReceived = void(PS4_FUNC *)(u32 handle, void* user, const SceMbusEvent* event);
struct SceMbusEventCallbacks {
    void(PS4_FUNC *overflow)(u32 handle, void* user);
    EventReceived received;
};

s32 PS4_FUNC sceMbusEventReceive(u32 handle, SceMbusEvent* out_events, u32 n_events, u32 unk1 /* maybe timeout? */);
s32 PS4_FUNC sceMbusEventCreate_(u32 mask, void* buffer, s32 capacity, const SceMbusEventCallbacks* callbacks, void* user);
s32 PS4_FUNC sceMbusEventDestroy(u32 handle);
void PS4_FUNC sceMbusDeviceInfoInit(void* info, u32 size);
void PS4_FUNC sceMbusEventCallbackFuncsInit_(void* callbacks, u32 size);
s32 PS4_FUNC sceMbusGetDeviceInfoByCondition_(const SceMbusDeviceInfo* condition, s32 start,
    SceMbusDeviceInfo* out_info, s32 capacity, s32* total, s32* generation, s32 stride);
s32 PS4_FUNC sceMbusGetDeviceInfo(u64 device_id, SceMbusDeviceInfo* info, u32 size);
s32 PS4_FUNC sceMbusBindDeviceWithUserId(u64 device_id, s32 user_id);
s32 PS4_FUNC sceMbusConvertToLocalDeviceId2(u64 device_id, u32 bus, u64* local_id);
s32 PS4_FUNC sceMbusSetAutoLoginMode(u32 mode);

}   // End namespace PS4::OS::Libs::SceMbus
