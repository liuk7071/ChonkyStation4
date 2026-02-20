#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNpMatching {

void init(Module& module);

static constexpr s32 SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID = 0x80550c0b;

using namespace OS::Np;
using SceNpMatching2ContextId               = u16;
using SceNpMatching2Event                   = u16;
using SceNpMatching2EventCause              = u8;
using SceNpMatching2ServerId                = u16;
using SceNpMatching2RequestId               = u32;
using SceNpMatching2WorldId                 = u32;
using SceNpMatching2LobbyId                 = u64;
using SceNpMatching2RoomId                  = u64;
using SceNpMatching2RoomPasswordSlotMask    = u64;
using SceNpMatching2RoomJoinedSlotMask      = u64;
using SceNpMatching2FlagAttr                = u32;
using SceNpMatching2RoomGroupId             = u8;
using SceNpMatching2AttributeId             = u16;
using SceNpMatching2Operator                = u8;

using SceNpMatching2RequestCallback = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2RequestId req_id, SceNpMatching2Event event, s32 error_code, const void* data, void* arg);
using SceNpMatching2ContextCallback = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2Event event, SceNpMatching2EventCause event_cause, s32 error_code, void* arg);

struct SceNpMatching2RequestOptParam {
    SceNpMatching2RequestCallback req_callback;
    void* req_callback_arg;
    u32 timeout;
    u16 app_req_id;
    u8 padding[2];
};

struct SceNpMatching2CreateContextParam {
    SceNpId* np_id;
    void* np_communication_id;
    void* np_passphrase;
    SceNpServiceLabel service_label;
    u64 size;
};

struct SceNpMatching2GetWorldInfoListRequest {
    SceNpMatching2ServerId server_id;
};

struct SceNpMatching2World {
    struct SceNpMatching2World* next;
    SceNpMatching2WorldId world_id;
    u32 n_lobbies;
    u32 max_lobby_members;
    u32 n_lobby_members;
    u32 n_rooms;
    u32 n_room_members;
    u8 reserved[3];
};

struct SceNpMatching2GetWorldInfoListResponse {
    SceNpMatching2World* worlds;
    u64 n_worlds;
};

struct SceNpMatching2Range {
    u32 start_idx;
    u32 total;
    u32 result_count;
    u8 padding[4];
};

struct SceNpMatching2RoomGroupInfo {
    SceNpMatching2RoomGroupId group_id;
    bool has_password;
    u8 padding[2];
    u32 n_slots;
    u32 n_group_members;
};

struct SceNpMatching2IntAttr {
    SceNpMatching2AttributeId id;
    u8 padding[2];
    u32 num;
};

struct SceNpMatching2BinAttr {
    SceNpMatching2AttributeId id;
    u8 padding[6];
    const void* ptr;
    size_t size;
};

struct SceNpMatching2RoomDataExternalA {
    struct SceNpMatching2RoomDataExternalA* next;
    u16 max_slots;
    u16 n_members;
    SceNpMatching2FlagAttr flag_attr;
    SceNpMatching2ServerId server_id;
    u8 padding[2];
    SceNpMatching2WorldId world_id;
    SceNpMatching2LobbyId lobby_id;
    SceNpMatching2RoomId room_id;
    SceNpMatching2RoomPasswordSlotMask password_slot_mask;
    SceNpMatching2RoomJoinedSlotMask joined_slot_mask;
    u16 n_public_slots;
    u16 n_private_slots;
    u16 n_free_public_slots;
    u16 n_free_private_slots;
    SceNpPeerAddressA owner;
    SceNpOnlineId owner_online_id;
    SceNpMatching2RoomGroupInfo* room_group;
    u64 n_room_groups;
    SceNpMatching2IntAttr* room_searchable_int_attr_external;
    u64 n_room_searchable_int_attr_external;
    SceNpMatching2BinAttr* room_searchable_bin_attr_external;
    u64 n_room_searchable_bin_attr_external;
    SceNpMatching2BinAttr* room_bin_attr_external;
    u64 n_room_bin_attr_external;
};

struct SceNpMatching2RangeFilter {
    u32 start_idx;
    u32 max;
};

struct SceNpMatching2IntSearchFilter {
    SceNpMatching2Operator search_op;
    u8 padding[7];
    SceNpMatching2IntAttr attr;
};

struct SceNpMatching2BinSearchFilter {
    SceNpMatching2Operator search_op;
    u8 padding[7];
    SceNpMatching2BinAttr attr;
};

struct SceNpMatching2SearchRoomRequest {
    s32 option;
    SceNpMatching2WorldId world_id;
    SceNpMatching2LobbyId lobby_id;
    SceNpMatching2RangeFilter range_filter;
    SceNpMatching2FlagAttr flag_filter;
    SceNpMatching2FlagAttr flag_attr;
    const SceNpMatching2IntSearchFilter* int_filter;
    u64 n_int_filters;
    const SceNpMatching2BinSearchFilter* bin_filter;
    u64 n_bin_filters;
    const SceNpMatching2AttributeId* attr_id;
    u64 n_attr_ids;
};

struct SceNpMatching2SearchRoomResponseA {
    SceNpMatching2Range range;
    SceNpMatching2RoomDataExternalA* room_data;
};

struct SceNpMatching2Context : SceObj {
    SceNpMatching2CreateContextParam param = {};
    
    SceNpMatching2ServerId server_id = 0;
    SceNpMatching2RequestOptParam default_req_param = {};

    SceNpMatching2World test_world;
    SceNpMatching2RoomDataExternalA test_room;
};

void checkCallback();

s32 PS4_FUNC sceNpMatching2CreateContext(const SceNpMatching2CreateContextParam* param, SceNpMatching2ContextId* ctx_id);
s32 PS4_FUNC sceNpMatching2SetDefaultRequestOptParam(const SceNpMatching2ContextId ctx_id, const SceNpMatching2RequestOptParam* param);
s32 PS4_FUNC sceNpMatching2RegisterContextCallback(SceNpMatching2ContextCallback callback, void* arg);
s32 PS4_FUNC sceNpMatching2ContextStart(const SceNpMatching2ContextId ctx_id, const u64 timeout);
s32 PS4_FUNC sceNpMatching2GetServerId(const SceNpMatching2ContextId ctx_id, SceNpMatching2ServerId* server_id);
s32 PS4_FUNC sceNpMatching2GetWorldInfoList(const SceNpMatching2ContextId ctx_id, const SceNpMatching2GetWorldInfoListRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2SearchRoom(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SearchRoomRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);

}   // End namespace PS4::OS::Libs::SceNpMatching