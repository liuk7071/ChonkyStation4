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
    module.addSymbolExport("GrQ9s4IrNaQ", "sceAudioOutGetPortState", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutGetPortState);
    module.addSymbolExport("QOQtbeDqsT4", "sceAudioOutOutput", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutOutput);
    module.addSymbolExport("w3PdaSTSwGE", "sceAudioOutOutputs", "libSceAudioOut", "libSceAudioOut", (void*)&sceAudioOutOutputs);

    module.addSymbolStub("s1--uE9mBFw", "sceAudioOutClose", "libSceAudioOut", "libSceAudioOut");
    module.addSymbolStub("b+uAV89IlxE", "sceAudioOutSetVolume", "libSceAudioOut", "libSceAudioOut");
    module.addSymbolStub("wVwPU50pS1c", "sceAudioOutSetMixLevelPadSpk", "libSceAudioOut", "libSceAudioOut");
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

static bool is_opened = false;
static s32 opened_handle = 0;
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
    // I stubbed it to only work for the first opened port for now...
    if (!is_opened) {
        is_opened = true;
        opened_handle = handle;

        SDL_AudioSpec desired, obtained;
        SDL_zero(desired);

        Helpers::debugAssert(sdl_format_map.contains(port->format), "sceAudioOutOpen: invalid format %d\n", port->format);
        const auto [format, n_channels] = sdl_format_map[port->format];
        port->sdl_format = format;
        port->n_channels = n_channels;

        desired.freq = 48000;
        desired.format = format;
        desired.channels = n_channels;
        desired.samples = len;
        desired.callback = NULL;

        dev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
        if (!dev) {
            Helpers::panic("Failed to open SDL audio device for playback\n");
        }

        SDL_PauseAudioDevice(dev, 0);
    }
    return handle;
}

s32 PS4_FUNC sceAudioOutGetPortState(s32 handle, SceAudioOutPortState* state) {
    log("sceAudioOutGetPortState(handle=%d, state=*%p)\n", handle, state);

    auto* port = PS4::OS::find<SceAudioOutPort>(handle);
    if (!port) {
        Helpers::panic("sceAudioOutGetPortState: could not find port with handle %d\n", handle);
    }

    state->output = 1;  // Primary output
    state->channel = port->n_channels;
    state->volume = 127;    // This is supposed to be set for PADSPK only
    state->reroute_counter = 0;
    return SCE_OK;
}

s32 PS4_FUNC sceAudioOutOutput(s32 handle, const void* ptr) {
    auto* port = PS4::OS::find<SceAudioOutPort>(handle);
    if (handle != opened_handle) return SCE_OK;

    if (ptr) {
        const size_t sample_size = port->sdl_format == AUDIO_F32 ? sizeof(float) : sizeof(s16);
        const auto size = port->len * sample_size * port->n_channels;
        SDL_QueueAudio(dev, ptr, size);

        while (SDL_GetQueuedAudioSize(dev) >= 4096 * 8 * port->n_channels) {
            SDL_Delay(1);
        }
        return port->len * port->n_channels;
    }
    else return 0;  // TODO: Should this return an error?
}

s32 PS4_FUNC sceAudioOutOutputs(SceAudioOutOutputParam* param, u32 num) {
    log("sceAudioOutOutputs(param=*%p, num=%d)\n", param, num);

    u32 samples = 0;
    for (int i = 0; i < num; i++) {
        samples += sceAudioOutOutput(param[i].handle, param[i].ptr);
    }
    return samples;
}

}   // End namespace PS4::OS::Libs::SceAudioOut