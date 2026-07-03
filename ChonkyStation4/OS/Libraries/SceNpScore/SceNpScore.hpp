#pragma once

#include <Common.hpp>
#include <OS/Np/NpTypes.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Libraries/SceRtc/SceRtc.hpp>


class Module;

namespace PS4::OS::Libs::SceNpScore {

void init(Module& module);

static constexpr s32 SCE_NP_SCORE_COMMENT_MAXLEN    = 63;
static constexpr s32 SCE_NP_SCORE_GAMEINFO_MAXSIZE  = 189;

using namespace OS::Np;
using SceNpScoreBoardId = u32;
using SceNpScoreRankNumber = u32;
using SceNpScorePcId = s32;
using SceNpScoreValue = s64;

struct SceNpScoreRankDataA {
    SceNpOnlineId online_id;
    u8 reserved0[16];
    u8 reserved1[49];
    u8 pad0[3];
    SceNpScorePcId pc_id;
    SceNpScoreRankNumber serial_rank;
    SceNpScoreRankNumber rank;
    SceNpScoreRankNumber highest_rank;
    s32 has_game_data;
    u8 pad1[4];
    SceNpScoreValue score_value;
    SceRtc::SceRtcTick record_date;
    SceNpAccountId account_id;
    u8 pad2[8];
};

struct SceNpScorePlayerRankDataA {
    s32 has_data;
    u8 pad0[4];
    SceNpScoreRankDataA rank_data;
};

struct SceNpScoreComment {
    char utf8_comment[SCE_NP_SCORE_COMMENT_MAXLEN + 1];
};

struct SceNpScoreGameInfo {
    size_t info_size;
    u8 data[SCE_NP_SCORE_GAMEINFO_MAXSIZE];
    u8 pad0[3];
};

s32 PS4_FUNC sceNpScoreCreateNpTitleCtx(SceNpServiceLabel service_label, const SceNpId* self_np_id);
s32 PS4_FUNC sceNpScoreCreateRequest(s32 ctx_id);
s32 PS4_FUNC sceNpScoreGetRankingByRangeAAsync(s32 req_id, SceNpScoreBoardId board_id, SceNpScoreRankNumber start_serial_rank, SceNpScoreRankDataA* rank_array, size_t rank_array_size, SceNpScoreComment* comment_array, size_t comment_array_size, SceNpScoreGameInfo* info_array, size_t info_array_size, size_t n_array, SceRtc::SceRtcTick* last_sort_date, SceNpScoreRankNumber* total_record, void* option);
s32 PS4_FUNC sceNpScoreGetRankingByAccountIdAsync(s32 req_id, SceNpScoreBoardId board_id, const SceNpAccountId* account_id_array, size_t account_id_array_size, SceNpScorePlayerRankDataA* rank_array, size_t rank_array_size, SceNpScoreComment* comment_array, size_t comment_array_size, SceNpScoreGameInfo* info_array, size_t info_array_size, size_t n_array, SceRtc::SceRtcTick* last_sort_date, SceNpScoreRankNumber* total_record, void* option);
s32 PS4_FUNC sceNpScoreRecordScoreAsync(s32 req_id, SceNpScoreBoardId board_id, SceNpScoreValue score, const SceNpScoreComment* comment, const SceNpScoreGameInfo* game_info, SceNpScoreRankNumber* tmp_rank, const SceRtc::SceRtcTick* compare_date, void* option);
s32 PS4_FUNC sceNpScorePollAsync(s32 req_id, s32* res);

}   // End namespace PS4::OS::Libs::SceNpScore