#include "SceNpMatching.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/SceNpMatching/NpEvents.hpp>
#include <deque>


namespace PS4::OS::Libs::SceNpMatching {

MAKE_LOG_FUNCTION(log, lib_sceNpMatching);

void init(Module& module) {
    module.addSymbolExport("YfmpW719rMo", "sceNpMatching2CreateContext", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2CreateContext);
    module.addSymbolExport("+8e7wXLmjds", "sceNpMatching2SetDefaultRequestOptParam", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SetDefaultRequestOptParam);
    module.addSymbolExport("fQQfP87I7hs", "sceNpMatching2RegisterContextCallback", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2RegisterContextCallback);
    module.addSymbolExport("7vjNQ6Z1op0", "sceNpMatching2ContextStart", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2ContextStart);
    module.addSymbolExport("LhCPctIICxQ", "sceNpMatching2GetServerId", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2GetServerId);
    module.addSymbolExport("rJNPJqDCpiI", "sceNpMatching2GetWorldInfoList", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2GetWorldInfoList);
    module.addSymbolExport("VqZX7POg2Mk", "sceNpMatching2SearchRoom", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SearchRoom);
    
    module.addSymbolStub("10t3e5+JPnU", "sceNpMatching2Initialize", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("p+2EnxmaAMM", "sceNpMatching2RegisterRoomEventCallback", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("uBESzz4CQws", "sceNpMatching2RegisterRoomMessageCallback", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("0UMeWRGnZKA", "sceNpMatching2RegisterSignalingCallback", "libSceNpMatching2", "libSceNpMatching2");
}

static s32 next_req_id = 1;

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

struct Request {
    Request() {
        req_id = OS::requestHandle();
    }

    SceNpMatching2RequestId req_id = 0;
    SceNpMatching2ContextId ctx_id = 0;
    SceNpMatching2RequestOptParam param = {};
    SceNpMatching2Event event = 0;
    std::shared_ptr<u8> data;
};
std::deque<Request> requests;

// Called by sceNpCheckCallback
void checkCallback() {
    while (ctx_events.size()) {
        auto event = ctx_events.front();
        ctx_events.pop_front();
        ctx_callback.func(event.ctx_id, event.event, event.event_cause, event.error_code, ctx_callback.arg);
    }

    while (requests.size()) {
        auto req = requests.front();
        requests.pop_front();
        req.param.req_callback(req.ctx_id, req.req_id, req.event, 0, (void*)req.data.get(), req.param.req_callback_arg);
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

s32 PS4_FUNC sceNpMatching2SetDefaultRequestOptParam(const SceNpMatching2ContextId ctx_id, const SceNpMatching2RequestOptParam* param) {
    log("sceNpMatching2SetDefaultRequestOptParam(ctx_id=%d, param=*%p)\n", ctx_id, param);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    ctx->default_req_param = *param;
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

    // Create a test world
    std::memset(&ctx->test_world, 0, sizeof(SceNpMatching2World));
    ctx->test_world.next = nullptr;
    ctx->test_world.world_id = OS::requestHandle();
    ctx->test_world.n_lobbies = 1;
    ctx->test_world.max_lobby_members = 16;
    ctx->test_world.n_lobby_members = 0;
    ctx->test_world.n_rooms = 1;
    ctx->test_world.n_room_members = 0;

    // Create a test room
    std::memset(&ctx->test_room, 0, sizeof(SceNpMatching2RoomDataExternalA));
    ctx->test_room.next = nullptr;
    ctx->test_room.max_slots = 16;
    ctx->test_room.n_members = 0;
    ctx->test_room.n_public_slots = 16;
    ctx->test_room.n_free_public_slots = 16;

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

s32 PS4_FUNC sceNpMatching2GetWorldInfoList(const SceNpMatching2ContextId ctx_id, const SceNpMatching2GetWorldInfoListRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2GetWorldInfoList(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    Request& req    = requests.emplace_back();
    req.ctx_id      = ctx->handle;
    req.param       = param ? *param : ctx->default_req_param;
    req.event       = SCE_NP_MATCHING2_REQUEST_EVENT_GET_WORLD_INFO_LIST;
    req.data        = std::make_shared<u8>(sizeof(SceNpMatching2GetWorldInfoListResponse));

    auto* res       = (SceNpMatching2GetWorldInfoListResponse*)req.data.get();
    res->worlds     = &ctx->test_world;
    res->n_worlds   = 1;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2SearchRoom(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SearchRoomRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2SearchRoom(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_SEARCH_ROOM_A;
    req.data = std::make_shared<u8>(sizeof(SceNpMatching2SearchRoomResponseA));

    auto* res = (SceNpMatching2SearchRoomResponseA*)req.data.get();
    res->range.start_idx    = 0;
    res->range.total        = 1;
    res->range.result_count = 1;
    res->room_data          = &ctx->test_room;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpMatching