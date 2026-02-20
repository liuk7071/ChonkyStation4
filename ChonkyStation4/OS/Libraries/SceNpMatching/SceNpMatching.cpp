#include "SceNpMatching.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/SceNpMatching/NpEvents.hpp>
#include <deque>


namespace PS4::OS::Libs::SceNpMatching {

MAKE_LOG_FUNCTION(log, lib_sceNpMatching);

void init(Module& module) {
    module.addSymbolExport("YfmpW719rMo", "sceNpMatching2CreateContext", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2CreateContext);
    module.addSymbolExport("fQQfP87I7hs", "sceNpMatching2RegisterContextCallback", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2RegisterContextCallback);
    module.addSymbolExport("7vjNQ6Z1op0", "sceNpMatching2ContextStart", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2ContextStart);
    module.addSymbolExport("LhCPctIICxQ", "sceNpMatching2GetServerId", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2GetServerId);
    
    module.addSymbolStub("10t3e5+JPnU", "sceNpMatching2Initialize", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("+8e7wXLmjds", "sceNpMatching2SetDefaultRequestOptParam", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("p+2EnxmaAMM", "sceNpMatching2RegisterRoomEventCallback", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("uBESzz4CQws", "sceNpMatching2RegisterRoomMessageCallback", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("0UMeWRGnZKA", "sceNpMatching2RegisterSignalingCallback", "libSceNpMatching2", "libSceNpMatching2");
}

struct {
    SceNpMatching2ContextCallback func = nullptr;
    void* arg = nullptr;
} ctx_callback;

struct ContextEvent {
    SceNpMatching2ContextId ctx_id = 0;
    SceNpMatching2Event event = 0;
    SceNpMatching2EventCause event_cause = 0;
    s32 error_code = 0;
};
std::deque<ContextEvent> ctx_events;

// Called by sceNpCheckCallback
void checkCallback() {
    while (ctx_events.size()) {
        auto event = ctx_events.front();
        ctx_events.pop_front();
        ctx_callback.func(event.ctx_id, event.event, event.event_cause, event.error_code, ctx_callback.arg);
    }
}

s32 PS4_FUNC sceNpMatching2CreateContext(const SceNpMatching2CreateContextParam* param, SceNpMatching2ContextId* ctx_id) {
    log("sceNpMatching2CreateContext(param=*%p, ctx_id=*%p)\n", param, ctx_id);

    auto* ctx = OS::make<SceNpMatching2Context>(true /* Request 16bit handle */);
    ctx->param = *param;
    ctx->server_id = 0x10;  // Random id

    *ctx_id = ctx->handle;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2RegisterContextCallback(SceNpMatching2ContextCallback callback, void* arg) {
    log("sceNpMatching2RegisterContextCallback(callback=%p, arg=%p)\n", callback, arg);

    if (ctx_callback.func) {
        Helpers::panic("sceNpMatching2RegisterContextCallback: called without unregistering previous callback\n");
    }

    ctx_callback.func = callback;
    ctx_callback.arg = arg;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2ContextStart(const SceNpMatching2ContextId ctx_id, const u64 timeout) {
    log("sceNpMatching2ContextStart(ctx_id=%d, timeout=%lld\n", ctx_id, timeout);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    // Send context start event
    ctx_events.push_back({ ctx_id, SCE_NP_MATCHING2_CONTEXT_EVENT_STARTED, SCE_NP_MATCHING2_EVENT_CAUSE_CONTEXT_ACTION, 0 });
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2GetServerId(const SceNpMatching2ContextId ctx_id, SceNpMatching2ServerId* server_id) {
    log("sceNpMatching2GetServerId(ctx_id=%d, server_id=*%p)\n", ctx_id, server_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    *server_id = ctx->server_id;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpMatching