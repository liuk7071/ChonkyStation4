#include "SceAudioOut.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <SDL.h>
#include <fstream>


namespace PS4::OS::Libs::SceAudioOut {

MAKE_LOG_FUNCTION(log, lib_sceAudioOut);

void init(Module& module) {
    module.addSymbolExport("JfEPXVxhFqA", "sceAudioOutInit", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutInit);
    module.addSymbolExport("ekNvsT22rsY", "sceAudioOutOpen", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutOpen);
    module.addSymbolExport("QOQtbeDqsT4", "sceAudioOutOutput", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutOutput);
}

std::ofstream output;
SDL_AudioDeviceID dev;

s32 PS4_FUNC sceAudioOutInit() {
    log("sceAudioOutInit()\n");
    
    // Init SDL audio
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);

    desired.freq = 48000;
    desired.format = AUDIO_F32;
    desired.channels = 2;
    desired.samples = 4096;
    desired.callback = NULL;

    dev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (!dev) {
        Helpers::panic("Failed to open SDL audio device for playback\n");
    }

    SDL_PauseAudioDevice(dev, 0);
    return SCE_OK;
}

s32 PS4_FUNC sceAudioOutOpen(Libs::SceUserService::SceUserServiceUserId uid, s32 type, s32 idx, u32 len, u32 freq, u32 param) {
    log("sceAudioOutOpen(uid=%d, type=%d, idx=%d, len=%d, freq=%d, param=0x%x)\n", uid, type, idx, len, freq, param);

    output.open("sample.bin", std::ios::binary);
    auto* port = PS4::OS::make<SceAudioOutPort>();
    port->device = (AudioVirtualDevice)type;
    port->len = len;
    port->freq = freq;
    port->format = param & 0xff;

    const auto handle = port->handle;
    log("Opened audio port %d: device=%s, len=%d, freq=%d, format=%d\n", handle, audioVirtualDeviceToStr(port->device).c_str(), port->len, port->freq, port->format);
    return handle;
}

s32 PS4_FUNC sceAudioOutOutput(s32 handle, const void* ptr) {
    auto* port = PS4::OS::find<SceAudioOutPort>(handle);

    const auto size = port->len * sizeof(float) * 2;
    SDL_QueueAudio(dev, ptr, size);
    
    while (SDL_GetQueuedAudioSize(dev) >= 4096 * 16) {
        SDL_Delay(1);
    }
    return port->len * 2;   // Stubbed to 2 channels
}

}   // End namespace PS4::OS::Libs::SceAudioOut