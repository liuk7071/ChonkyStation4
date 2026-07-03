#pragma once

#include <PSN/Providers/Provider.hpp>

#define ASIO_STANDALONE
#include <asio.hpp>


namespace PSN {

class ChonkyNetProvider : public Provider {
public:
    void init() override;
    bool login(PS4::OS::User::User* user) override;

    void getRankingByRangeAsync(SceNpRequest* req, SceNpScoreBoardId board_id, SceNpScoreRankNumber start_rank, SceNpScoreRankDataA* rank_array, size_t n_ranks, SceNpScoreComment* comment_array, size_t n_comments, SceNpScoreGameInfo* info_array, size_t n_info, size_t n_array, SceRtcTick* last_sort_date, SceNpScoreRankNumber* n_total_ranks) override;
    void recordScoreAsync(SceNpRequest* req, SceNpScoreBoardId board_id, SceNpScoreValue score, const SceNpScoreComment* comment, const SceNpScoreGameInfo* game_info, SceNpScoreRankNumber* tmp_rank, const SceRtcTick* compare_date) override;

private:
    asio::io_context io;
    std::unique_ptr<asio::ip::tcp::socket> sock = nullptr;
};

}   // End namespace PSN