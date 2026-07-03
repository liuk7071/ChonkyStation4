#include "SceNpScore.hpp"
#include <Logger.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/SceNpManager/SceNpManager.hpp>   // For SceNpRequest
#include <PSN/PSN.hpp>


namespace PS4::OS::Libs::SceNpScore {

MAKE_LOG_FUNCTION(log, lib_sceNpScore);

void init(Module& module) {
    module.addSymbolExport("KnNA1TEgtBI", "sceNpScoreCreateNpTitleCtx", "libSceNpScore", "libSceNpScore", (void*)&sceNpScoreCreateNpTitleCtx);
    module.addSymbolExport("gW8qyjYrUbk", "sceNpScoreCreateRequest", "libSceNpScore", "libSceNpScore", (void*)&sceNpScoreCreateRequest);
    module.addSymbolExport("y5ja7WI05rs", "sceNpScoreGetRankingByRangeAAsync", "libSceNpScore", "libSceNpScore", (void*)&sceNpScoreGetRankingByRangeAAsync);
    module.addSymbolExport("dRszNNyGWkw", "sceNpScoreGetRankingByAccountIdAsync", "libSceNpScore", "libSceNpScore", (void*)&sceNpScoreGetRankingByAccountIdAsync);
    module.addSymbolExport("ANJssPz3mY0", "sceNpScoreRecordScoreAsync", "libSceNpScore", "libSceNpScore", (void*)&sceNpScoreRecordScoreAsync);
    module.addSymbolExport("m1DfNRstkSQ", "sceNpScorePollAsync", "libSceNpScore", "libSceNpScore", (void*)&sceNpScorePollAsync);
    
    module.addSymbolStub("GWnWQNXZH5M", "sceNpScoreCreateNpTitleCtxA", "libSceNpScore", "libSceNpScore", 1);
    module.addSymbolStub("8kuIzUw6utQ", "sceNpScoreGetFriendsRanking", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("9mZEgoiEq6Y", "sceNpScoreGetRankingByNpId", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("KBHxDjyk-jA", "sceNpScoreGetRankingByRange", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("dK8-SgYf6r4", "sceNpScoreDeleteRequest", "libSceNpScore", "libSceNpScore", 0);
}

s32 PS4_FUNC sceNpScoreCreateNpTitleCtx(SceNpServiceLabel service_label, const SceNpId* self_np_id) {
    log("sceNpScoreCreateNpTitleCtx(service_label=%d, self_np_id=*%p)\n", service_label, self_np_id);

    // TODO: Proper context creation
    return 1;   // handle
}

s32 PS4_FUNC sceNpScoreCreateRequest(s32 ctx_id) {
    log("sceNpScoreCreateRequest(ctx_id=%d) [forwarding to sceNpCreateRequest]\n", ctx_id);
    return SceNpManager::sceNpCreateRequest();
}

s32 PS4_FUNC sceNpScoreGetRankingByRangeAAsync(s32 req_id, SceNpScoreBoardId board_id, SceNpScoreRankNumber start_serial_rank, SceNpScoreRankDataA* rank_array, size_t rank_array_size, SceNpScoreComment* comment_array, size_t comment_array_size, SceNpScoreGameInfo* info_array, size_t info_array_size, size_t n_array, SceRtc::SceRtcTick* last_sort_date, SceNpScoreRankNumber* total_record, void* option) {
    log("sceNpScoreGetRankingByRangeAAsync(req_id=%d, board_id=%d, start_serial_rank=%d, rank_array=*%p, rank_array_size=%lld, comment_array=*%p, comment_array_size=%lld, info_array=*%p, info_array_size=%lld, n_array=%lld, last_sort_date=*%p, total_record=*%p, option=%p)\n", req_id, board_id, start_serial_rank, rank_array, rank_array_size, comment_array, comment_array_size, info_array, info_array_size, n_array, last_sort_date, total_record, option);

    auto* req = OS::find<SceNpManager::SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    req->is_async = true;
    PSN::psn->getRankingByRangeAsync(req, board_id, start_serial_rank, rank_array, rank_array_size, comment_array, comment_array_size, info_array, info_array_size, n_array, last_sort_date, total_record);
    return SCE_OK;
}

s32 PS4_FUNC sceNpScoreGetRankingByAccountIdAsync(s32 req_id, SceNpScoreBoardId board_id, const SceNpAccountId* account_id_array, size_t account_id_array_size, SceNpScorePlayerRankDataA* rank_array, size_t rank_array_size, SceNpScoreComment* comment_array, size_t comment_array_size, SceNpScoreGameInfo* info_array, size_t info_array_size, size_t n_array, SceRtc::SceRtcTick* last_sort_date, SceNpScoreRankNumber* total_record, void* option) {
    log("sceNpScoreGetRankingByAccountIdAsync(req_id=%d, board_id=%d, account_id_array=*%p, account_id_array_size=%lld, rank_array=*%p, rank_array_size=%lld, comment_array=*%p, comment_array_size=%lld, info_array=*%p, info_array_size=%lld, n_array=%lld, last_sort_date=*%p, total_record=*%p, option=%p)\n", req_id, board_id, account_id_array, account_id_array_size, rank_array, rank_array_size, comment_array, comment_array_size, info_array, info_array_size, n_array, last_sort_date, total_record, option);

    auto* req = OS::find<SceNpManager::SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;
    
    // TODO
    req->is_async = true;
    req->finish(SCE_OK);
    return SCE_OK;
}

s32 PS4_FUNC sceNpScoreRecordScoreAsync(s32 req_id, SceNpScoreBoardId board_id, SceNpScoreValue score, const SceNpScoreComment* comment, const SceNpScoreGameInfo* game_info, SceNpScoreRankNumber* tmp_rank, const SceRtc::SceRtcTick* compare_date, void* option) {
    log("sceNpScoreRecordScoreAsync(req_id=%d, board_id=%d, score=%d, comment=*%p, game_info=*%, tmp_rank=*%p, compare_date=*%p, option=%p)\n", req_id, board_id, score, comment, game_info, tmp_rank, compare_date, option);
    
    auto* req = OS::find<SceNpManager::SceNpRequest>(req_id);
    if (!req) return SCE_NP_ERROR_REQUEST_NOT_FOUND;

    req->is_async = true;
    PSN::psn->recordScoreAsync(req, board_id, score, comment, game_info, tmp_rank, compare_date);
    return SCE_OK;
}

s32 PS4_FUNC sceNpScorePollAsync(s32 req_id, s32* res) {
    log("sceNpScorePollAsync(req_id=%d, res=*%p) [forwarding to sceNpPollAsync]\n", req_id, res);
    return SceNpManager::sceNpPollAsync(req_id, res);
}

}   // End namespace PS4::OS::Libs::SceNpScore