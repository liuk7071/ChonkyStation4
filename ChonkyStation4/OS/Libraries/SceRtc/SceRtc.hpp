#pragma once

#include <Common.hpp>


class Module;

namespace PS4::OS::Libs::SceRtc {

void init(Module& module);

struct SceRtcTick {
    u64 tick;
};

struct SceRtcDateTime {
    u16 year;
    u16 month;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
    u32 microsecond;
};

s32 PS4_FUNC sceRtcGetCurrentTick(SceRtcTick* tick);
s32 PS4_FUNC sceRtcGetTick(const SceRtcDateTime* time, SceRtcTick* tick);

}   // End namespace PS4::OS::Libs::SceRtc