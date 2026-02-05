#include "SceRandom.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>


namespace PS4::OS::Libs::SceRandom {

MAKE_LOG_FUNCTION(log, lib_sceRandom);

void init(Module& module) {
    module.addSymbolExport("PI7jIZj4pcE", "sceRandomGetRandomNumber", "libSceRandom", "libSceRandom", (void*)&sceRandomGetRandomNumber);

    std::srand(std::time(nullptr));
}

s32 PS4_FUNC sceRandomGetRandomNumber(void* buf, size_t size) {
    log("sceRandomGetRandomNumer(buf=*%p, size=0x%llx)\n", buf, size);

    for (u8* ptr = (u8*)buf; ptr < (u8*)buf + size; ptr++) {
        *ptr = std::rand() & 0xff;
    }

    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceRandom