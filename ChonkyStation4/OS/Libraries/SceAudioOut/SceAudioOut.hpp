#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>   // For SceUserServiceUserId
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceAudioOut {

void init(Module& module);

enum AudioVirtualDevice {
    Main,
    BGM,
    Voice,
    Personal,
    PadSpeaker,
    Aux = 127
};

inline std::string audioVirtualDeviceToStr(AudioVirtualDevice dev) {
    switch (dev) {
    case AudioVirtualDevice::Main:          return "Main";
    case AudioVirtualDevice::BGM:           return "BGM";
    case AudioVirtualDevice::Voice:         return "Voice";
    case AudioVirtualDevice::Personal:      return "Personal";
    case AudioVirtualDevice::PadSpeaker:    return "PadSpeaker";
    case AudioVirtualDevice::Aux:           return "Aux";
    }
    Helpers::panic("audioVirtualDeviceToStr: unreachable\n");
}

struct SceAudioOutOutputParam {
    s32 handle;
    const void* ptr;
};

struct SceAudioOutPortState {
    u16 output;
    u8 channel;
    u8 reserved1[1];
    s16 volume;
    u16 reroute_counter;
    u64 flag;
    u64 reserved2[2];
};

struct SceAudioOutPort : SceObj {
    AudioVirtualDevice device;
    u32 len;
    u32 freq;
    u32 format;
    u32 n_channels;
    u32 sdl_format;
};

s32 PS4_FUNC sceAudioOutInit();
s32 PS4_FUNC sceAudioOutOpen(Libs::SceUserService::SceUserServiceUserId uid, s32 type, s32 idx, u32 len, u32 freq, u32 param);
s32 PS4_FUNC sceAudioOutGetPortState(s32 handle, SceAudioOutPortState* state);
s32 PS4_FUNC sceAudioOutOutput(s32 handle, const void* ptr);
s32 PS4_FUNC sceAudioOutOutputs(SceAudioOutOutputParam* param, u32 num);

}   // End namespace PS4::OS::Libs::SceAudioOut