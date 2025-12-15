#include "SceAudioOut.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <SDL.h>
#include <fstream>
#include <unordered_map>


namespace PS4::OS::Libs::SceAudioOut {

MAKE_LOG_FUNCTION(log, lib_sceAudioOut);

void init(Module& module) {
    module.addSymbolExport("JfEPXVxhFqA", "sceAudioOutInit", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutInit);
    module.addSymbolExport("ekNvsT22rsY", "sceAudioOutOpen", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutOpen);
    module.addSymbolExport("QOQtbeDqsT4", "sceAudioOutOutput", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutOutput);

    module.addSymbolStub("b+uAV89IlxE", "sceAudioOutSetVolume", "libSceAudioOut", "libSceAudioOut");
}

SDL_AudioDeviceID dev;

s32 PS4_FUNC sceAudioOutInit() {
    log("sceAudioOutInit()\n");
    return SCE_OK;
}

static std::unordered_map<s32, std::pair<s32, s32>> sdl_format_map = {
    { 0, { AUDIO_S16, 1 }},
    { 1, { AUDIO_S16, 2 }},
    { 2, { AUDIO_S16, 8 }},
    { 3, { AUDIO_F32, 1 }},
    { 4, { AUDIO_F32, 2 }},
    { 5, { AUDIO_F32, 8 }},
    { 6, { AUDIO_S16, 8 }},
    { 7, { AUDIO_F32, 8 }},
};

s32 PS4_FUNC sceAudioOutOpen(Libs::SceUserService::SceUserServiceUserId uid, s32 type, s32 idx, u32 len, u32 freq, u32 param) {
    log("sceAudioOutOpen(uid=%d, type=%d, idx=%d, len=%d, freq=%d, param=0x%x)\n", uid, type, idx, len, freq, param);

    auto* port = PS4::OS::make<SceAudioOutPort>();
    port->device = (AudioVirtualDevice)type;
    port->len = len;
    port->freq = freq;
    port->format = param & 0xff;

    const auto handle = port->handle;
    log("Opened audio port %d: device=%s, len=%d, freq=%d, format=%d\n", handle, audioVirtualDeviceToStr(port->device).c_str(), port->len, port->freq, port->format);

    // Init SDL audio
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);

    Helpers::debugAssert(sdl_format_map.contains(port->format), "sceAudioOutOpen: invalid format %d\n", port->format);
    const auto [format, n_channels] = sdl_format_map[port->format];
    port->sdl_format = format;
    port->n_channels = n_channels;
    
    desired.freq = 48000;
    desired.format = format;
    desired.channels = n_channels;
    desired.samples = 4096;
    desired.callback = NULL;

    dev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (!dev) {
        Helpers::panic("Failed to open SDL audio device for playback\n");
    }

    SDL_PauseAudioDevice(dev, 0);
    return handle;
}

s32 PS4_FUNC sceAudioOutOutput(s32 handle, const void* ptr) {
    auto* port = PS4::OS::find<SceAudioOutPort>(handle);

    const size_t sample_size = port->sdl_format == AUDIO_F32 ? sizeof(float) : sizeof(s16);
    const auto size = port->len * sample_size * port->n_channels;
    SDL_QueueAudio(dev, ptr, size);
    
    while (SDL_GetQueuedAudioSize(dev) >= 4096 * 8 * port->n_channels) {
        SDL_Delay(1);
    }
    return port->len * port->n_channels;
}

}   // End namespace PS4::OS::Libs::SceAudioOut