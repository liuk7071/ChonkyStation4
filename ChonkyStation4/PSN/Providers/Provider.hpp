#pragma once

#include <OS/UserManagement.hpp>
#include <OS/Libraries/SceNpManager/SceNpManager.hpp>
#include <OS/Libraries/SceNpScore/SceNpScore.hpp>
#include <OS/Libraries/SceRtc/SceRtc.hpp>


namespace PSN {

using namespace PS4::OS::Libs::SceNpManager;
using namespace PS4::OS::Libs::SceNpScore;
using namespace PS4::OS::Libs::SceRtc;

class Provider {
public:
    virtual void init() = 0;
    virtual bool login(PS4::OS::User::User* user) = 0;

    virtual void getRankingByRangeAsync(SceNpRequest* req, SceNpScoreBoardId board_id, SceNpScoreRankNumber start_rank, SceNpScoreRankDataA* rank_array, size_t n_ranks, SceNpScoreComment* comment_array, size_t n_comments, SceNpScoreGameInfo* info_array, size_t n_info, size_t n_array, SceRtcTick* last_sort_date, SceNpScoreRankNumber* n_total_ranks) = 0;
    virtual void recordScoreAsync(SceNpRequest* req, SceNpScoreBoardId board_id, SceNpScoreValue score, const SceNpScoreComment* comment, const SceNpScoreGameInfo* game_info, SceNpScoreRankNumber* tmp_rank, const SceRtcTick* compare_date) = 0;
};

}   // End namespace PSN