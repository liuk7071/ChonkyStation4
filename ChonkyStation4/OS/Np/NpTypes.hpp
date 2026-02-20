#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>


namespace PS4::OS::Np {

static constexpr s32 SCE_NP_ERROR_SIGNED_OUT        = 0x80550006;
static constexpr s32 SCE_NP_ERROR_REQUEST_NOT_FOUND = 0x80550014;

static constexpr s32 SCE_NP_ONLINEID_MAX_LENGTH = 16;

static constexpr s32 SCE_NP_PLUS_FEATURE_REALTIME_MULTIPLAY = 1;

static constexpr s32 SCE_NP_PLATFORM_TYPE_NONE      = 0;
static constexpr s32 SCE_NP_PLATFORM_TYPE_PS3       = 1;
static constexpr s32 SCE_NP_PLATFORM_TYPE_VITA      = 2;
static constexpr s32 SCE_NP_PLATFORM_TYPE_PS4       = 3;

using SceNpServiceLabel = u32;
using SceNpAccountId    = u64;
using SceNpPlatformType = s32;

enum class SceNpState {
    SCE_NP_STATE_UNKNOWN = 0,
    SCE_NP_STATE_SIGNED_OUT,
    SCE_NP_STATE_SIGNED_IN
};

struct SceNpOnlineId {
    char data[SCE_NP_ONLINEID_MAX_LENGTH];
    char term;
    char dummy[3];
};

struct SceNpId {
    SceNpOnlineId handle;
    u8 opt[8];
    u8 reserved[8];
};

struct SceNpPeerAddressA {
    SceNpAccountId account_id;
    SceNpPlatformType platform;
    char padding[4];
};

struct SceNpParentalControlInfo {
    bool content_restriction;
    bool chat_restriction;
    bool ugc_restriction;   // User-generated media
};

struct SceNpCheckPlusParameter {
    size_t size;
    OS::Libs::SceUserService::SceUserServiceUserId user_id;
    char padding[4];
    u64 features;
    u8 reserved[32];
};

struct SceNpCheckPlusResult {
    bool authorized;
    u8 reserved[32];
};

}   // End namespace PS4::OS::Np