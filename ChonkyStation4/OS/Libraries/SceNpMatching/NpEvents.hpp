#pragma once

#include <Common.hpp>

namespace PS4::OS::Libs::SceNpMatching {

// SceNpMatching2Event
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_WORLD_INFO_LIST                     = 0x0002;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_DATA_EXTERNAL                  = 0x0004;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_LOBBY_INFO_LIST                     = 0x0006;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SET_USER_INFO                           = 0x0007;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_LEAVE_ROOM                              = 0x0103;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GRANT_ROOM_OWNER                        = 0x0104;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_KICKOUT_ROOM_MEMBER                     = 0x0105;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SEND_ROOM_CHAT_MESSAGE                  = 0x0107;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SEND_ROOM_MESSAGE                       = 0x0108;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_DATA_INTERNAL                  = 0x0109;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_DATA_INTERNAL                  = 0x010a;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SET_ROOM_MEMBER_DATA_INTERNAL           = 0x010b;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SET_SIGNALING_OPT_PARAM                 = 0x010d;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_JOIN_LOBBY                              = 0x0201;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_LEAVE_LOBBY                             = 0x0202;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SEND_LOBBY_CHAT_MESSAGE                 = 0x0203;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SET_LOBBY_MEMBER_DATA_INTERNAL          = 0x0205;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SIGNALING_GET_PING_INFO                 = 0x0e01;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_KICKEDOUT                                  = 0x1103;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_ROOM_DESTROYED                             = 0x1104;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_ROOM_OWNER_CHANGED                         = 0x1105;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_ROOM_DATA_INTERNAL                 = 0x1106;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_SIGNALING_OPT_PARAM                = 0x1108;
static constexpr s32 SCE_NP_MATCHING2_LOBBY_EVENT_LOBBY_DESTROYED                           = 0x3203;
static constexpr s32 SCE_NP_MATCHING2_SIGNALING_EVENT_DEAD                                  = 0x5101;
static constexpr s32 SCE_NP_MATCHING2_SIGNALING_EVENT_ESTABLISHED                           = 0x5102;
static constexpr s32 SCE_NP_MATCHING2_SIGNALING_EVENT_NETINFO_RESULT                        = 0x5103;
static constexpr s32 SCE_NP_MATCHING2_CONTEXT_EVENT_START_OVER                              = 0x6f01;
static constexpr s32 SCE_NP_MATCHING2_CONTEXT_EVENT_STARTED                                 = 0x6f02;
static constexpr s32 SCE_NP_MATCHING2_CONTEXT_EVENT_STOPPED                                 = 0x6f03;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_MEMBER_DATA_EXTERNAL_LIST_A    = 0x7003;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_DATA_EXTERNAL_LIST_A           = 0x7005;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_USER_INFO_LIST_A                    = 0x7008;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_CREATE_JOIN_ROOM_A                      = 0x7101;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_JOIN_ROOM_A                             = 0x7102;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_SEARCH_ROOM_A                           = 0x7106;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_ROOM_MEMBER_DATA_INTERNAL_A         = 0x710c;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_LOBBY_MEMBER_DATA_INTERNAL_A        = 0x7206;
static constexpr s32 SCE_NP_MATCHING2_REQUEST_EVENT_GET_LOBBY_MEMBER_DATA_INTERNAL_LIST_A   = 0x7207;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_MEMBER_JOINED_A                            = 0x8101;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_MEMBER_LEFT_A                              = 0x8102;
static constexpr s32 SCE_NP_MATCHING2_ROOM_EVENT_UPDATED_ROOM_MEMBER_DATA_INTERNAL_A        = 0x8107;
static constexpr s32 SCE_NP_MATCHING2_ROOM_MSG_EVENT_CHAT_MESSAGE_A                         = 0x9101;
static constexpr s32 SCE_NP_MATCHING2_ROOM_MSG_EVENT_MESSAGE_A                              = 0x9102;
static constexpr s32 SCE_NP_MATCHING2_LOBBY_EVENT_MEMBER_JOINED_A                           = 0xa201;
static constexpr s32 SCE_NP_MATCHING2_LOBBY_EVENT_MEMBER_LEFT_A                             = 0xa202;
static constexpr s32 SCE_NP_MATCHING2_LOBBY_EVENT_UPDATED_LOBBY_MEMBER_DATA_INTERNAL_A      = 0xa204;
static constexpr s32 SCE_NP_MATCHING2_LOBBY_MSG_EVENT_CHAT_MESSAGE_A                        = 0xb201;

// SceNpMatching2EventCause
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_LEAVE_ACTION = 1;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_KICKOUT_ACTION = 2;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_GRANT_OWNER_ACTION = 3;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_SERVER_OPERATION = 4;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_MEMBER_DISAPPEARED = 5;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_SERVER_INTERNAL = 6;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_CONNECTION_ERROR = 7;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_NP_SIGNED_OUT = 8;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_SYSTEM_ERROR = 9;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_CONTEXT_ERROR = 10;
static constexpr s32 SCE_NP_MATCHING2_EVENT_CAUSE_CONTEXT_ACTION = 11;

}   // End namespace PS4::OS::Libs::SceNpMatching