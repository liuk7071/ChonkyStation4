#include "SceRtc.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <ctime>


namespace PS4::OS::Libs::SceRtc {

MAKE_LOG_FUNCTION(log, lib_sceRtc);

void init(Module& module) {
    module.addSymbolExport("18B2NS1y9UU", "sceRtcGetCurrentTick", "libSceRtc", "libSceRtc", (void*)&sceRtcGetCurrentTick);
    module.addSymbolExport("zO9UL3qIINQ", "sceRtcGetCurrentNetworkTick", "libSceRtc", "libSceRtc", (void*)&sceRtcGetCurrentNetworkTick);
    module.addSymbolExport("8w-H19ip48I", "sceRtcGetTick", "libSceRtc", "libSceRtc", (void*)&sceRtcGetTick);
    module.addSymbolExport("ueega6v3GUw", "sceRtcSetTick", "libSceRtc", "libSceRtc", (void*)&sceRtcSetTick);
    module.addSymbolExport("mn-tf4QiFzk", "sceRtcTickAddMinutes", "libSceRtc", "libSceRtc", (void*)&sceRtcTickAddMinutes);
    module.addSymbolExport("BtqmpTRXHgk", "sceRtcGetTime_t", "libSceRtc", "libSceRtc", (void*)&sceRtcGetTime_t);
    module.addSymbolExport("ZPD1YOKI+Kw", "sceRtcGetCurrentClockLocalTime", "libSceRtc", "libSceRtc", (void*)&sceRtcGetCurrentClockLocalTime);
}

s32 PS4_FUNC sceRtcGetCurrentTick(SceRtcTick* tick) {
    log("sceRtcGetCurrentTick(tick=*%p)\n", tick);

    using namespace std::chrono;
    tick->tick = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    return SCE_OK;
}

s32 PS4_FUNC sceRtcGetCurrentNetworkTick(SceRtcTick* tick) {
    log("sceRtcGetCurrentNetworkTick(tick=*%p)\n", tick);

    // Same as sceRtcGetCurrentTick for now
    using namespace std::chrono;
    tick->tick = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    return SCE_OK;
}


s32 PS4_FUNC sceRtcGetTick(const SceRtcDateTime* time, SceRtcTick* tick) {
    log("sceRtcGetTick(time=*%p, tick=*%p)\n", time, tick);

    std::tm tm_time;
    tm_time.tm_year = time->year;
    tm_time.tm_mon  = time->month;
    tm_time.tm_mday = time->day;
    tm_time.tm_hour = time->hour;
    tm_time.tm_min  = time->minute;
    tm_time.tm_sec  = time->second;

    std::time_t seconds;
#ifdef _WIN32
    seconds = _mkgmtime(&tm_time);
#else
    seconds = timegm(&tm_time);
#endif

    tick->tick = seconds * 1'000'000 + time->microsecond;
    return SCE_OK;
}

s32 PS4_FUNC sceRtcSetTick(SceRtcDateTime* time, const SceRtcTick* tick) {
    log("sceRtcSetTick(time=*%p, tick=*%p)\n", time, tick);

    std::time_t seconds = tick->tick / 1'000'000;
    s32 microseconds = tick->tick % 1'000'000;

    std::tm tm_time;
#ifdef _WIN32
    gmtime_s(&tm_time, &seconds);
#else
    gmtime_s(&seconds, &tm_time);
#endif

    time->year          = tm_time.tm_year; 
    time->month         = tm_time.tm_mon;
    time->day           = tm_time.tm_mday; 
    time->hour          = tm_time.tm_hour;
    time->minute        = tm_time.tm_min;  
    time->second        = tm_time.tm_sec;
    time->microsecond   = microseconds;
    return SCE_OK;
}

s32 PS4_FUNC sceRtcTickAddMinutes(SceRtcTick* out_tick, const SceRtcTick* in_tick, s64 min) {
    log("sceRtcTickAddMinutes(out_tick=*%p, in_tick=*%p, min=%lld)\n", out_tick, in_tick, min);

    // Convert minutes to ticks (microseconds)
    using namespace std::chrono;
    const u64 us = duration_cast<microseconds>(minutes(min)).count();
    out_tick->tick = in_tick->tick + us;
    return SCE_OK;
}

s32 PS4_FUNC sceRtcGetTime_t(const SceRtcDateTime* sce_time, u64* time) {
    log("sceRtcGetTime_t(sce_time=*%p, time=*%p)\n", sce_time, time);

    SceRtcTick tick;
    sceRtcGetTick(sce_time, &tick);
    *time = tick.tick / 1'000'000;
    return SCE_OK;
}

s32 PS4_FUNC sceRtcGetCurrentClockLocalTime(SceRtcDateTime* time) {
    log("sceRtcGetCurrentClockLocalTime(time=*%p)\n", time);

    SceRtcTick tick;
    sceRtcGetCurrentTick(&tick);
    sceRtcSetTick(time, &tick);
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceRtc