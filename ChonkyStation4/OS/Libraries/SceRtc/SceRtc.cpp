#include "SceRtc.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <ctime>


namespace PS4::OS::Libs::SceRtc {

MAKE_LOG_FUNCTION(log, lib_sceRtc);

void init(Module& module) {
    module.addSymbolExport("18B2NS1y9UU", "sceRtcGetCurrentTick", "libSceRtc", "libSceRtc", (void*)&sceRtcGetCurrentTick);
    module.addSymbolExport("8w-H19ip48I", "sceRtcGetTick", "libSceRtc", "libSceRtc", (void*)&sceRtcGetTick);
}

s32 PS4_FUNC sceRtcGetCurrentTick(SceRtcTick* tick) {
    log("sceRtcGetCurrentTick(tick=*%p)\n", tick);

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

}   // End namespace PS4::OS::Libs::SceRtc