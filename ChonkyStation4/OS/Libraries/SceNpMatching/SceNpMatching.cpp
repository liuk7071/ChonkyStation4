#include "SceNpMatching.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/SceNpMatching/NpEvents.hpp>
#include <OS/UserManagement.hpp>
#include <deque>
#include <mutex>


namespace PS4::OS::Libs::SceNpMatching {

MAKE_LOG_FUNCTION(log, lib_sceNpMatching);

void init(Module& module) {
    module.addSymbolExport("YfmpW719rMo", "sceNpMatching2CreateContext", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2CreateContext);
    module.addSymbolExport("+8e7wXLmjds", "sceNpMatching2SetDefaultRequestOptParam", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SetDefaultRequestOptParam);
    module.addSymbolExport("fQQfP87I7hs", "sceNpMatching2RegisterContextCallback", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2RegisterContextCallback);
    module.addSymbolExport("p+2EnxmaAMM", "sceNpMatching2RegisterRoomEventCallback", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2RegisterRoomEventCallback);
    module.addSymbolExport("uBESzz4CQws", "sceNpMatching2RegisterRoomMessageCallback", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2RegisterRoomMessageCallback);
    module.addSymbolExport("7vjNQ6Z1op0", "sceNpMatching2ContextStart", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2ContextStart);
    module.addSymbolExport("LhCPctIICxQ", "sceNpMatching2GetServerId", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2GetServerId);
    module.addSymbolExport("rJNPJqDCpiI", "sceNpMatching2GetWorldInfoList", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2GetWorldInfoList);
    module.addSymbolExport("VqZX7POg2Mk", "sceNpMatching2SearchRoom", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SearchRoom);
    module.addSymbolExport("zCWZmXXN600", "sceNpMatching2CreateJoinRoom", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2CreateJoinRoom);
    module.addSymbolExport("meEjIdbjAA0", "sceNpMatching2SetUserInfo", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SetUserInfo);
    module.addSymbolExport("S9D8JSYIrjE", "sceNpMatching2SetRoomDataInternal", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SetRoomDataInternal);
    module.addSymbolExport("q7GK98-nYSE", "sceNpMatching2SetRoomDataExternal", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SetRoomDataExternal);
    module.addSymbolExport("Iw2h0Jrrb5U", "sceNpMatching2SendRoomMessage", "libSceNpMatching2", "libSceNpMatching2", (void*)&sceNpMatching2SendRoomMessage);
    
    module.addSymbolStub("10t3e5+JPnU", "sceNpMatching2Initialize", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("0UMeWRGnZKA", "sceNpMatching2RegisterSignalingCallback", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("Mqp3lJ+sjy4", "sceNpMatching2Terminate", "libSceNpMatching2", "libSceNpMatching2");
}

static s32 next_req_id = 1;
std::mutex req_mtx;

struct {
    SceNpMatching2ContextCallback func = nullptr;
    void* arg = nullptr;
} ctx_callback;

struct {
    SceNpMatching2RoomEventCallback func = nullptr;
    void* arg = nullptr;
} room_callback;

struct {
    SceNpMatching2RoomMessageCallback func = nullptr;
    void* arg = nullptr;
} room_message_callback;

struct ContextEvent {
    SceNpMatching2ContextId ctx_id = 0;
    SceNpMatching2Event event = 0;
    SceNpMatching2EventCause event_cause = 0;
    s32 error_code = 0;
};
std::deque<ContextEvent> ctx_events;

struct RoomEvent {
    SceNpMatching2ContextId ctx_id = 0;
    SceNpMatching2RoomId room_id = 0;
    SceNpMatching2Event event = 0;
    void* data = nullptr;
};
std::deque<RoomEvent> room_events;

struct RoomMessageEvent {
    SceNpMatching2ContextId ctx_id = 0;
    SceNpMatching2RoomId room_id = 0;
    SceNpMatching2RoomMemberId src_member_id = 0;
    SceNpMatching2Event event = 0;
    void* data = nullptr;
};
std::deque<RoomMessageEvent> room_message_events;

struct Request {
    Request() {
        req_id = OS::requestHandle();
    }

    SceNpMatching2RequestId req_id = 0;
    SceNpMatching2ContextId ctx_id = 0;
    SceNpMatching2RequestOptParam param = {};
    SceNpMatching2Event event = 0;
    std::vector<u8> data;
};
std::deque<Request> requests;

// Called by sceNpCheckCallback
void checkCallback() {
    while (ctx_events.size()) {
        ContextEvent event;
        {
            const auto lk = std::unique_lock<std::mutex>(req_mtx);
            event = ctx_events.front();
            ctx_events.pop_front();
        }
        ctx_callback.func(event.ctx_id, event.event, event.event_cause, event.error_code, ctx_callback.arg);
    }

    while (requests.size()) {
        Request req;
        {
            const auto lk = std::unique_lock<std::mutex>(req_mtx);
            req = requests.front();
            requests.pop_front();
        }
        req.param.req_callback(req.ctx_id, req.req_id, req.event, 0, (void*)req.data.data(), req.param.req_callback_arg);
    }

    while (room_events.size()) {
        RoomEvent event;
        {
            const auto lk = std::unique_lock<std::mutex>(req_mtx);
            event = room_events.front();
            room_events.pop_front();
        }
        room_callback.func(event.ctx_id, event.room_id, event.event, event.data, room_callback.arg);
    }

    while (room_message_events.size()) {
        RoomMessageEvent event;
        {
            const auto lk = std::unique_lock<std::mutex>(req_mtx);
            event = room_message_events.front();
            room_message_events.pop_front();
        }
        room_message_callback.func(event.ctx_id, event.room_id, event.src_member_id, event.event, event.data, room_message_callback.arg);
    }
}

s32 PS4_FUNC sceNpMatching2CreateContext(const SceNpMatching2CreateContextParam* param, SceNpMatching2ContextId* ctx_id) {
    log("sceNpMatching2CreateContext(param=*%p, ctx_id=*%p)\n", param, ctx_id);

    auto* ctx = OS::make<SceNpMatching2Context>(true /* Request 16bit handle */);
    ctx->param = *param;
    ctx->server_id = 0x80552C42;  // Random id

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

s32 PS4_FUNC sceNpMatching2RegisterRoomEventCallback(const SceNpMatching2ContextId ctx_id, SceNpMatching2RoomEventCallback callback, void* arg) {
    log("sceNpMatching2RegisterRoomEventCallback(ctx_id=%d, callback=%p, arg=%p)\n", ctx_id, callback, arg);

    if (room_callback.func) {
        Helpers::panic("sceNpMatching2RegisterRoomEventCallback: called without unregistering previous callback\n");
    }

    room_callback.func = callback;
    room_callback.arg = arg;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2RegisterRoomMessageCallback(const SceNpMatching2ContextId ctx_id, SceNpMatching2RoomMessageCallback callback, void* arg) {
    log("sceNpMatching2RegisterRoomMessageCallback(ctx_id=%d, callback=%p, arg=%p)\n", ctx_id, callback, arg);

    if (room_message_callback.func) {
        Helpers::panic("sceNpMatching2RegisterRoomMessageCallback: called without unregistering previous callback\n");
    }

    room_message_callback.func = callback;
    room_message_callback.arg = arg;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2ContextStart(const SceNpMatching2ContextId ctx_id, const u64 timeout) {
    log("sceNpMatching2ContextStart(ctx_id=%d, timeout=%lld)\n", ctx_id, timeout);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    // Create a test world
    std::memset(&ctx->test_world, 0, sizeof(SceNpMatching2World));
    ctx->test_world.next = nullptr;
    ctx->test_world.world_id = 65537;
    ctx->test_world.n_lobbies = 1;
    ctx->test_world.max_lobby_members = 16;
    ctx->test_world.n_lobby_members = 1;
    ctx->test_world.n_rooms = 1;
    ctx->test_world.n_room_members = 1;

    // Create a test room
    std::memset(&ctx->test_room, 0, sizeof(SceNpMatching2RoomDataExternalA));
    ctx->test_room.next = nullptr;
    ctx->test_room.max_slots = 16;
    ctx->test_room.server_id = ctx->server_id;
    ctx->test_room.world_id = ctx->test_world.world_id;
    ctx->test_room.lobby_id = 100;
    ctx->test_room.room_id = OS::requestHandle();
    ctx->test_room.n_members = 0;
    ctx->test_room.n_public_slots = 16;
    ctx->test_room.n_free_public_slots = 16;
    ctx->test_room.room_searchable_int_attr_external = new SceNpMatching2IntAttr();
    ctx->test_room.room_searchable_int_attr_external->id = 0x4c;
    ctx->test_room.room_searchable_int_attr_external->num = 1;
    ctx->test_room.n_room_searchable_int_attr_external = 1;
    ctx->test_room.room_searchable_bin_attr_external = new SceNpMatching2BinAttr();
    ctx->test_room.room_searchable_bin_attr_external->id = 0x55;
    ctx->test_room.room_searchable_bin_attr_external->ptr = (void*)0x12345;
    ctx->test_room.room_searchable_bin_attr_external->size = 10;
    ctx->test_room.n_room_bin_attr_external = 1;

    // Send context start event
    const auto lk = std::unique_lock<std::mutex>(req_mtx);
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

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req    = requests.emplace_back();
    req.ctx_id      = ctx->handle;
    req.param       = param ? *param : ctx->default_req_param;
    req.event       = SCE_NP_MATCHING2_REQUEST_EVENT_GET_WORLD_INFO_LIST;
    req.data        = std::vector<u8>(sizeof(SceNpMatching2GetWorldInfoListResponse));

    auto* res       = (SceNpMatching2GetWorldInfoListResponse*)req.data.data();
    res->worlds     = &ctx->test_world;
    res->n_worlds   = 1;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2SearchRoom(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SearchRoomRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2SearchRoom(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);
    log("---------------------------------------------\n");
    log("option        : 0x%x\n", req_param->option);
    log("world_id      : 0x%llx\n", req_param->world_id);
    log("lobby_id      : 0x%llx\n", req_param->lobby_id);
    log("flag_filter   : 0x%x\n", req_param->flag_filter);
    log("flag_attr     : 0x%x\n", req_param->flag_attr);
    for (int i = 0; i < req_param->n_int_filters; i++) {
        log("int_filter#%d:\n", i);
        log("  search_op : 0x%x\n", req_param->int_filter[i].search_op);
        log("  attr.id   : 0x%x\n", req_param->int_filter[i].attr.id);
        log("  attr.num  : 0x%x\n", req_param->int_filter[i].attr.num);
    }
    for (int i = 0; i < req_param->n_bin_filters; i++) {
        log("bin_filter#%d:\n", i);
        log("  search_op : 0x%x\n", req_param->bin_filter[i].search_op);
        log("  attr.id   : 0x%x\n", req_param->bin_filter[i].attr.id);
        log("  attr.ptr  : %p\n", req_param->bin_filter[i].attr.ptr);
        log("  attr.size : 0x%llx\n", req_param->bin_filter[i].attr.size);
    }
    for (int i = 0; i < req_param->n_attr_ids; i++) {
        log("attr_id#%d    : 0x%x\n", i, req_param->attr_id[i]);
    }
    log("---------------------------------------------\n");

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_SEARCH_ROOM_A;
    //req.event = 0x106;
    req.data = std::vector<u8>(sizeof(SceNpMatching2SearchRoomResponseA));

    auto* res = (SceNpMatching2SearchRoomResponseA*)req.data.data();
    res->range.start_idx    = 0;
    res->range.total        = 1;
    res->range.result_count = 1;
    res->room_data          = &ctx->test_room;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2CreateJoinRoom(const SceNpMatching2ContextId ctx_id, const SceNpMatching2CreateJoinRoomRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2CreateJoinRoom(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);
    log("room_bin_attr_internal     : %p\n", req_param->room_bin_attr_internal);
    log("n_room_bin_attr_internal   : %p\n", req_param->n_room_bin_attr_internal);
    for (int i = 0; i < req_param->n_room_bin_attr_internal; i++) {
        log("room_bin_attr_internal#%d:\n", i);
        log("  room_bin_attr_internal.id   : 0x%x\n",     req_param->room_bin_attr_internal[i].id);
        log("  room_bin_attr_internal.ptr  : %p\n",       req_param->room_bin_attr_internal[i].ptr);
        log("  room_bin_attr_internal.size : 0x%llx\n",   req_param->room_bin_attr_internal[i].size);
    }

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_CREATE_JOIN_ROOM;
    req.data = std::vector<u8>(sizeof(SceNpMatching2CreateJoinRoomResponse));

    auto* res = (SceNpMatching2CreateJoinRoomResponse*)req.data.data();
    auto* room_data_internal = new SceNpMatching2RoomDataInternal();
    res->room_data_internal = room_data_internal;
    std::memset(room_data_internal, 0, sizeof(SceNpMatching2RoomDataInternal));
    //room_data_internal->room_groups = new SceNpMatching2RoomGroup[1];
    //room_data_internal->n_groups = 1;
    //room_data_internal->room_bin_attr_internal = new SceNpMatching2RoomBinAttrInternal[1];
    //room_data_internal->n_room_bin_attr_internal = 1;
    room_data_internal->n_public_slots = 16;
    room_data_internal->n_private_slots = 16;
    room_data_internal->n_open_public_slots = 15;
    room_data_internal->n_open_private_slots = 16;
    room_data_internal->max_slots = 16;
    room_data_internal->server_id = 0x80552C42;
    room_data_internal->world_id = req_param->world_id;
    room_data_internal->lobby_id = req_param->lobby_id;
    room_data_internal->room_id = 1234;
    room_data_internal->password_slot_mask = 0;
    room_data_internal->joined_slot_mask = 3;
    room_data_internal->room_groups = nullptr;
    room_data_internal->n_groups = 0;
    room_data_internal->flag_attr = req_param->flag_attr;
    room_data_internal->n_room_bin_attr_internal = req_param->n_room_bin_attr_internal;
    room_data_internal->room_bin_attr_internal = new SceNpMatching2RoomBinAttrInternal[req_param->n_room_bin_attr_internal];
    for (int i = 0; i < req_param->n_room_bin_attr_internal; i++) {
        room_data_internal->room_bin_attr_internal[i].data = req_param->room_bin_attr_internal[i];
    }

    res->member_list.n_members = 2;
    res->member_list.members = new SceNpMatching2RoomMemberDataInternal[2]; // TODO: free later
    std::memset(res->member_list.members, 0, sizeof(SceNpMatching2RoomMemberDataInternal) * 2);
    
    res->member_list.members[0].next = &res->member_list.members[1];

    res->member_list.members[0].member_id = 1;
    std::strcpy(res->member_list.members[0].np_id.handle.data, User::current->online_id.c_str());
    res->member_list.members[1].member_id = 2;
    std::strcpy(res->member_list.members[1].np_id.handle.data, "test_user");


    res->member_list.me = &res->member_list.members[0];
    res->member_list.owner = &res->member_list.members[0];

    RoomEvent& room_event = room_events.emplace_back();
    room_event.ctx_id = ctx_id;
    room_event.room_id = 1234;
    room_event.event = SCE_NP_MATCHING2_ROOM_EVENT_MEMBER_JOINED;
    room_event.data = nullptr;

    room_event = room_events.emplace_back();
    room_event.ctx_id = ctx_id;
    room_event.room_id = 1234;
    room_event.event = SCE_NP_MATCHING2_ROOM_EVENT_ROOM_OWNER_CHANGED;
    room_event.data = nullptr;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2SetUserInfo(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SetUserInfoRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2SetUserInfo(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_SET_USER_INFO;
    req.data = std::vector<u8>(1);  // No response, technically nullptr should be passed

    RoomEvent& room_event = room_events.emplace_back();
    room_event.ctx_id = ctx_id;
    room_event.room_id = 1234;
    room_event.event = SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_ROOM_MEMBER_DATA_INTERNAL;
    room_event.data = nullptr;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2SetRoomDataInternal(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SetRoomDataInternalRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2SetRoomDataInternal(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_DATA_INTERNAL;
    req.data = std::vector<u8>(1);  // No response, technically nullptr should be passed

    RoomEvent& room_event = room_events.emplace_back();
    room_event.ctx_id = ctx_id;
    room_event.room_id = 1234;
    room_event.event = SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_ROOM_DATA_INTERNAL;
    room_event.data = nullptr;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2SetRoomDataExternal(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SetRoomDataExternalRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2SetRoomDataExternal(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_DATA_EXTERNAL;
    req.data = std::vector<u8>(1);  // No response, technically nullptr should be passed

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

s32 PS4_FUNC sceNpMatching2SendRoomMessage(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SendRoomMessageRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id) {
    log("sceNpMatching2SendRoomMessage(ctx_id=%d, req_param=*%p, param=*%p, assigned_req_id=*%p)\n", ctx_id, req_param, param, assigned_req_id);

    auto* ctx = OS::find<SceNpMatching2Context>(ctx_id);
    if (!ctx) return SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID;

    const auto lk = std::unique_lock<std::mutex>(req_mtx);
    Request& req = requests.emplace_back();
    req.ctx_id = ctx->handle;
    req.param = param ? *param : ctx->default_req_param;
    req.event = SCE_NP_MATCHING2_REQUEST_EVENT_SEND_ROOM_MESSAGE;
    req.data = std::vector<u8>(1);  // No response, technically nullptr should be passed

    RoomMessageEvent& room_message_event = room_message_events.emplace_back();
    room_message_event.ctx_id = ctx_id;
    room_message_event.room_id = 1234;
    room_message_event.src_member_id = 1;
    room_message_event.event = SCE_NP_MATCHING2_ROOM_MSG_EVENT_MESSAGE;
    room_message_event.data = nullptr;

    *assigned_req_id = req.req_id;
    return SCE_OK;
}

}   // End namespace PS4::OS::Libs::SceNpMatching