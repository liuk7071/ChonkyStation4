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
s32 PS4_FUNC sceRtcGetCurrentNetworkTick(SceRtcTick* tick);
s32 PS4_FUNC sceRtcGetTick(const SceRtcDateTime* time, SceRtcTick* tick);
s32 PS4_FUNC sceRtcSetTick(SceRtcDateTime* time, const SceRtcTick* tick);
s32 PS4_FUNC sceRtcTickAddMinutes(SceRtcTick* out_tick, const SceRtcTick* in_tick, s64 min);
s32 PS4_FUNC sceRtcGetTime_t(const SceRtcDateTime* sce_time, u64* time);
s32 PS4_FUNC sceRtcGetCurrentClockLocalTime(SceRtcDateTime* time);

}   // End namespace PS4::OS::Libs::SceRtc