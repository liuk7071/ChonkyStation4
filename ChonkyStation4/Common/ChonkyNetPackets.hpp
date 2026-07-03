#pragma once

#include <Common.hpp>


namespace ChonkyNet {

static constexpr s32 CHONKYNET_VERSION = 0x0001;
static constexpr s32 CHONKYNET_OK = 0;
static constexpr size_t CHONKYNET_SESSION_TOKEN_LENGTH = 32;

enum PacketType : u32 {
    Login = 1,
    AuthorizeSession,
    GetRankingByRange,
    RecordScore
};

struct PacketHeader {
    size_t size;
    PacketType type;
};

struct ResponseHeader {
    s32 err;
};

static constexpr s32 ERROR_SESSION_NOT_AUTHORIZED = -100;

// -------- Login --------

static constexpr s32 LOGIN_ERROR_OUTDATED_CLIENT = -1;
static constexpr s32 LOGIN_ERROR_INVALID_USERNAME_OR_PASSWORD = -2;

struct LoginPacket {
    u32 client_version;
    char username[16];
    char password[64];
};


struct LoginResponse {
    ResponseHeader header;
    char session_token[CHONKYNET_SESSION_TOKEN_LENGTH];
};

// -------- Authorize Session --------

static constexpr s32 AUTHORIZE_SESSION_ERROR_OUTDATED_CLIENT = -1;
static constexpr s32 AUTHORIZE_SESSION_ERROR_INVALID_TOKEN = -2;

struct AuthorizeSessionPacket {
    u32 client_version;
    char username[16];
    char session_token[CHONKYNET_SESSION_TOKEN_LENGTH];
};

struct AuthorizeSessionResponse {
    ResponseHeader header;
    u64 account_id;
};

// -------- Get Ranking by Range --------

using SceNpAccountId = u64;
using SceNpScoreBoardId = u32;
using SceNpScoreRankNumber = u32;
using SceNpScorePcId = s32;
using SceNpScoreValue = s64;

struct GetRankingByRangePacket {
    char title_id[9];
    SceNpScoreBoardId board_id;
    SceNpScoreRankNumber starting_rank;
    size_t n_ranks;
};

struct GetRankingByRangeResponse {
    ResponseHeader header;
    size_t n_obtained_ranks;
    size_t n_total_ranks;
};

struct RankingInfo {
    char online_id[16];
    SceNpScorePcId pc_id;
    SceNpScoreValue score;
    SceNpScoreRankNumber rank;
    SceNpScoreRankNumber highest_rank;
    SceNpAccountId account_id;
    // TODO: Comment, game data
};

// -------- Record Score --------

struct RecordScorePacket {
    char title_id[9];
    SceNpScoreBoardId board_id;
    SceNpScoreValue score;
    SceNpAccountId account_id;
    // TODO: Comment, game data
};

struct RecordScoreResponse {
    ResponseHeader header;
    SceNpScoreRankNumber temp_rank;
};

}   // End namespace ChonkyNet