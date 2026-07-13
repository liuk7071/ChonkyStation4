#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/Libraries/SceRtc/SceRtc.hpp>
#include <OS/SceObj.hpp>


class Module;

namespace PS4::OS::Libs::SceNpMatching {

void init(Module& module);

static constexpr s32 SCE_NP_MATCHING2_ERROR_INVALID_CONTEXT_ID = 0x80550c0b;

static constexpr size_t SCE_NP_MATCHING2_SESSION_PASSWORD_SIZE  = 8;
static constexpr size_t SCE_NP_MATCHING2_GROUP_LABEL_SIZE       = 8;

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
using SceNpMatching2TeamId                  = u8;
using SceNpMatching2SignalingType           = u8;
using SceNpMatching2SignalingFlag           = u8;
using SceNpMatching2RoomMemberId            = u16;
using SceNpMatching2NatType                 = u8;

using SceNpMatching2RequestCallback     = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2RequestId req_id, SceNpMatching2Event event, s32 error_code, const void* data, void* arg);
using SceNpMatching2ContextCallback     = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2Event event, SceNpMatching2EventCause event_cause, s32 error_code, void* arg);
using SceNpMatching2RoomEventCallback   = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2RoomId room_id, SceNpMatching2Event event, const void* data, void* arg);
using SceNpMatching2RoomMessageCallback = PS4_FUNC void (*)(SceNpMatching2ContextId ctx_id, SceNpMatching2RoomId room_id, SceNpMatching2RoomMemberId src_member_id, SceNpMatching2Event event, const void* data, void* arg);

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

struct SceNpMatching2SessionPassword {
    unsigned char data[SCE_NP_MATCHING2_SESSION_PASSWORD_SIZE];
};

struct SceNpMatching2GroupLabel {
    unsigned char data[SCE_NP_MATCHING2_GROUP_LABEL_SIZE];
};

struct SceNpMatching2RoomGroupConfig {
    u32 n_slots;    // slots per group
    bool with_label;
    SceNpMatching2GroupLabel label;
    bool with_password;
    unsigned char padding[2];
};

struct SceNpMatching2SignalingOptParam {
    SceNpMatching2SignalingType type;
    SceNpMatching2SignalingFlag flag;
    SceNpMatching2RoomMemberId hub_member_id;
    u8 padding[4];
};

struct SceNpMatching2RoomGroup {
    SceNpMatching2RoomGroupId group_id;
    bool with_password;
    bool with_label;
    u8 pad[1];
    SceNpMatching2GroupLabel label;
    u32 n_slots;
    u32 n_curr_group_members;
};

struct SceNpMatching2RoomBinAttrInternal {
    SceRtc::SceRtcTick update_date;
    SceNpMatching2RoomMemberId last_updated_member_id;
    u8 pad[6];
    SceNpMatching2BinAttr data;
};

struct SceNpMatching2RoomDataInternal {
    u16 n_public_slots;
    u16 n_private_slots;
    u16 n_open_public_slots;
    u16 n_open_private_slots;
    u16 max_slots;
    SceNpMatching2ServerId server_id;
    SceNpMatching2WorldId world_id;
    SceNpMatching2LobbyId lobby_id;
    SceNpMatching2RoomId room_id;
    SceNpMatching2RoomPasswordSlotMask password_slot_mask;
    SceNpMatching2RoomJoinedSlotMask joined_slot_mask;
    SceNpMatching2RoomGroup* room_groups;
    u64 n_groups;
    SceNpMatching2FlagAttr flag_attr;
    u8 pad[4];
    SceNpMatching2RoomBinAttrInternal* room_bin_attr_internal;
    u64 n_room_bin_attr_internal;
};

struct SceNpMatching2RoomMemberBinAttrInternal {
    SceRtc::SceRtcTick update_date;
    SceNpMatching2BinAttr data;
};

struct SceNpMatching2RoomMemberDataInternal {
    SceNpMatching2RoomMemberDataInternal* next;
    SceRtc::SceRtcTick join_date;
    SceNpId np_id;
    u8 pad[4];
    SceNpMatching2RoomMemberId member_id;
    SceNpMatching2TeamId team_id;
    SceNpMatching2NatType nat_type;
    SceNpMatching2FlagAttr flag_attr;
    SceNpMatching2RoomGroup* room_group;
    SceNpMatching2RoomMemberBinAttrInternal* room_member_bin_attr_internal;
    u64 n_room_member_bin_attr_internal;
};

struct SceNpMatching2RoomMemberDataInternalList {
    SceNpMatching2RoomMemberDataInternal* members;
    u64 n_members;
    SceNpMatching2RoomMemberDataInternal* me;
    SceNpMatching2RoomMemberDataInternal* owner;
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

struct SceNpMatching2CreateJoinRoomRequest {
    u16 max_slots;
    SceNpMatching2TeamId default_team_id;
    u8 pad[5];
    SceNpMatching2FlagAttr flag_attr;
    SceNpMatching2WorldId world_id;
    SceNpMatching2LobbyId lobby_id;
    const SceNpMatching2SessionPassword* room_password;
    const SceNpMatching2RoomPasswordSlotMask* password_slot_mask;
    const SceNpMatching2RoomGroupConfig* group_config;
    u64 n_group_configs;
    const SceNpMatching2GroupLabel* required_group_label;
    const SceNpOnlineId* allowed_users;
    u64 n_allowed_users;
    const SceNpOnlineId* blocked_users;
    u64 n_blocked_users;
    const SceNpMatching2BinAttr* room_bin_attr_internal;
    u64 n_room_bin_attr_internal;
    const SceNpMatching2IntAttr* room_searchable_int_attr_external;
    u64 n_room_searchable_int_attr_external;
    const SceNpMatching2BinAttr* room_searchable_bin_attr_external;
    u64 n_room_searchable_bin_attr_external;
    const SceNpMatching2BinAttr* room_bin_attr_external;
    u64 n_room_bin_attr_external;
    const SceNpMatching2BinAttr* room_member_bin_attr_internal;
    u64 n_room_member_bin_attr_internal;
    const SceNpMatching2SignalingOptParam* signaling_opt_param;
};

struct SceNpMatching2CreateJoinRoomResponse {
    const SceNpMatching2RoomDataInternal* room_data_internal;
    SceNpMatching2RoomMemberDataInternalList member_list;
};

struct SceNpMatching2SetUserInfoRequest;
struct SceNpMatching2SetRoomDataInternalRequest;
struct SceNpMatching2SetRoomDataExternalRequest;
struct SceNpMatching2SendRoomMessageRequest;

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
s32 PS4_FUNC sceNpMatching2RegisterRoomEventCallback(const SceNpMatching2ContextId ctx_id, SceNpMatching2RoomEventCallback callback, void* arg);
s32 PS4_FUNC sceNpMatching2RegisterRoomMessageCallback(const SceNpMatching2ContextId ctx_id, SceNpMatching2RoomMessageCallback callback, void* arg);
s32 PS4_FUNC sceNpMatching2ContextStart(const SceNpMatching2ContextId ctx_id, const u64 timeout);
s32 PS4_FUNC sceNpMatching2GetServerId(const SceNpMatching2ContextId ctx_id, SceNpMatching2ServerId* server_id);
s32 PS4_FUNC sceNpMatching2GetWorldInfoList(const SceNpMatching2ContextId ctx_id, const SceNpMatching2GetWorldInfoListRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2SearchRoom(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SearchRoomRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2CreateJoinRoom(const SceNpMatching2ContextId ctx_id, const SceNpMatching2CreateJoinRoomRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2SetUserInfo(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SetUserInfoRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2SetRoomDataInternal(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SetRoomDataInternalRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2SetRoomDataExternal(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SetRoomDataExternalRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);
s32 PS4_FUNC sceNpMatching2SendRoomMessage(const SceNpMatching2ContextId ctx_id, const SceNpMatching2SendRoomMessageRequest* req_param, const SceNpMatching2RequestOptParam* param, SceNpMatching2RequestId* assigned_req_id);

}   // End namespace PS4::OS::Libs::SceNpMatching