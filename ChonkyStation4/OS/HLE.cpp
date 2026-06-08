#include "HLE.hpp"
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>
#include <OS/Libraries/SceSystemService/SceSystemService.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Libraries/SceNpManager/SceNpManager.hpp>
#include <OS/Libraries/SceNpMatching/SceNpMatching.hpp>
#include <OS/Libraries/SceSysmodule/SceSysmodule.hpp>
#include <OS/Libraries/SceSaveData/SceSaveData.hpp>
#include <OS/Libraries/SceSaveDataDialog/SceSaveDataDialog.hpp>
#include <OS/Libraries/SceNpTrophy/SceNpTrophy.hpp>
#include <OS/Libraries/ScePad/ScePad.hpp>
#include <OS/Libraries/SceAudioOut/SceAudioOut.hpp>
#include <OS/Libraries/ScePlayGo/ScePlayGo.hpp>
#include <OS/Libraries/SceRtc/SceRtc.hpp>
#include <OS/Libraries/SceNet/SceNet.hpp>
#include <OS/Libraries/SceRandom/SceRandom.hpp>
#include <OS/Libraries/SceVideodec/SceVideodec.hpp>
#include <OS/Libraries/SceAjm/SceAjm.hpp>
#include <OS/Libraries/SceAppContent/SceAppContent.hpp>
#include <OS/Libraries/SceZlib/SceZlib.hpp>


// Stub until we implement audio input
s32 PS4_FUNC sceAudioInInput(s32 handle, void* ptr) {
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}

// Move to sceSsl file later
struct SceSslData {
    char* ptr;
    size_t size;
};

struct SceSslCaCerts {
    SceSslData* cert;
    size_t n_certs;
    void* pool;
};

s32 PS4_FUNC sceSslGetCaCerts(s32 ctx_id, SceSslCaCerts* certs) {
    printf("sceSslGetCaCerts(ctx_id=%d, certs=*%p)\n", ctx_id, certs);

    // Some games check that at least some data is present. Return a dummy value.
    constexpr std::string dummy_data = "chonky";
    certs->cert = new SceSslData();
    certs->cert[0].ptr = new char[1_KB];
    std::strcpy(certs->cert[0].ptr, dummy_data.data());
    certs->cert[0].size = dummy_data.length();

    certs->n_certs = 1;
    certs->pool = nullptr;
    return SCE_OK;
}

s32 PS4_FUNC sceNetCtlGetState(s32* state) {
    //printf("sceNetCtlGetState()\n");
    *state = 0; // Disconnected
    return SCE_OK;
}

namespace PS4::OS::HLE {

// Create a dummy HLE module that only contains the symbol exports for HLE functions
std::shared_ptr<Module> buildHLEModule() {
    std::shared_ptr<Module> module = std::make_shared<Module>();
    module->filename = "HLE";

    PS4::OS::Libs::Kernel::init(*module);
    PS4::OS::Libs::SceVideoOut::init(*module);
    PS4::OS::Libs::SceGnmDriver::init(*module);
    PS4::OS::Libs::SceSystemService::init(*module);
    PS4::OS::Libs::SceUserService::init(*module);
    PS4::OS::Libs::SceNpManager::init(*module);
    PS4::OS::Libs::SceNpMatching::init(*module);
    PS4::OS::Libs::SceSysmodule::init(*module);
    PS4::OS::Libs::SceSaveData::init(*module);
    PS4::OS::Libs::SceSaveDataDialog::init(*module);
    PS4::OS::Libs::SceNpTrophy::init(*module);
    PS4::OS::Libs::ScePad::init(*module);
    PS4::OS::Libs::SceAudioOut::init(*module);
    PS4::OS::Libs::ScePlayGo::init(*module);
    PS4::OS::Libs::SceRtc::init(*module);
    PS4::OS::Libs::SceNet::init(*module);
    PS4::OS::Libs::SceRandom::init(*module);
    PS4::OS::Libs::SceVideodec::init(*module);
    PS4::OS::Libs::SceAjm::init(*module);
    PS4::OS::Libs::SceAppContent::init(*module);
    PS4::OS::Libs::SceZlib::init(*module);

    // libSceScreenShot
    module->addSymbolStub("2xxUtuC-RzE", "sceScreenShotEnable", "libSceScreenShot", "libSceScreenShot");
    module->addSymbolStub("73WQ4Jj0nJI", "sceScreenShotSetOverlayImageWithOrigin", "libSceScreenShot", "libSceScreenShot");
    module->addSymbolStub("G7KlmIYFIZc", "sceScreenShotSetParam", "libSceScreenShot", "libSceScreenShot");
    module->addSymbolStub("ahHhOf+QNkQ", "sceScreenShotSetOverlayImage", "libSceScreenShot", "libSceScreenShot");
    module->addSymbolStub("tIYf0W5VTi8", "sceScreenShotDisable", "libSceScreenShot", "libSceScreenShot");
    
    // libSceSharePlay
    module->addSymbolStub("isruqthpYcw", "sceSharePlayInitialize", "libSceSharePlay", "libSceSharePlay");
    module->addSymbolStub("co2NCj--pnc", "sceSharePlaySetProhibition", "libSceSharePlay", "libSceSharePlay");
    module->addSymbolStub("OOrLKB0bSDs", "sceSharePlayGetCurrentConnectionInfo", "libSceSharePlay", "libSceSharePlay");
    
    // libSceMsgDialog
    module->addSymbolStub("lDqxaY1UbEo", "sceMsgDialogInitialize", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("b06Hh0DPEaE", "sceMsgDialogOpen", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("6fIC3XKt2k0", "sceMsgDialogUpdateStatus", "libSceMsgDialog", "libSceMsgDialog", 3);
    module->addSymbolStub("CWVW78Qc3fI", "sceMsgDialogGetStatus", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("Lr8ovHH9l6A", "sceMsgDialogGetResult", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("HTrcDKlFKuM", "sceMsgDialogClose", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("ePw-kqZmelo", "sceMsgDialogTerminate", "libSceMsgDialog", "libSceMsgDialog");
    
    // libSceLoginDialog
    module->addSymbolStub("2rc+egSfb5A", "sceLoginDialogUpdateStatus", "libSceLoginDialog", "libSceLoginDialog", 3);
    module->addSymbolStub("Btkx21f1M8k", "sceLoginDialogGetResult", "libSceLoginDialog", "libSceLoginDialog");
    module->addSymbolStub("vMQJRUKsf3U", "sceLoginDialogTerminate", "libSceLoginDialog", "libSceLoginDialog");

    // libSceCommonDialog
    module->addSymbolStub("uoUpLGNkygk", "sceCommonDialogInitialize", "libSceCommonDialog", "libSceCommonDialog");
    module->addSymbolStub("BQ3tey0JmQM", "sceCommonDialogIsUsed", "libSceCommonDialog", "libSceCommonDialog", false);
    
    // libSceNpScore
    module->addSymbolStub("KnNA1TEgtBI", "sceNpScoreCreateNpTitleCtx", "libSceNpScore", "libSceNpScore", 1);
    module->addSymbolStub("GWnWQNXZH5M", "sceNpScoreCreateNpTitleCtxA", "libSceNpScore", "libSceNpScore", 1);
    module->addSymbolStub("gW8qyjYrUbk", "sceNpScoreCreateRequest", "libSceNpScore", "libSceNpScore", 1);
    module->addSymbolStub("ANJssPz3mY0", "sceNpScoreRecordScoreAsync", "libSceNpScore", "libSceNpScore", 0);
    module->addSymbolStub("8kuIzUw6utQ", "sceNpScoreGetFriendsRanking", "libSceNpScore", "libSceNpScore", 0);
    module->addSymbolStub("9mZEgoiEq6Y", "sceNpScoreGetRankingByNpId", "libSceNpScore", "libSceNpScore", 0);
    module->addSymbolStub("KBHxDjyk-jA", "sceNpScoreGetRankingByRange", "libSceNpScore", "libSceNpScore", 0);
    module->addSymbolStub("dK8-SgYf6r4", "sceNpScoreDeleteRequest", "libSceNpScore", "libSceNpScore", 0);
    module->addSymbolStub("m1DfNRstkSQ", "sceNpScorePollAsync", "libSceNpScore", "libSceNpScore", 0);
    
    // libSceNpParty
    module->addSymbolStub("lhYCTQmBkds", "sceNpPartyInitialize", "libSceNpParty", "libSceNpParty");
    module->addSymbolStub("kA88gbv71ao", "sceNpPartyRegisterHandler", "libSceNpParty", "libSceNpParty");
    module->addSymbolStub("3e4k2mzLkmc", "sceNpPartyCheckCallback", "libSceNpParty", "libSceNpParty");
    
    // libSceErrorDialog
    module->addSymbolStub("I88KChlynSs", "sceErrorDialogInitialize", "libSceErrorDialog", "libSceErrorDialog", 0);
    module->addSymbolStub("M2ZF-ClLhgY", "sceErrorDialogOpen", "libSceErrorDialog", "libSceErrorDialog", 0);
    module->addSymbolStub("WWiGuh9XfgQ", "sceErrorDialogUpdateStatus", "libSceErrorDialog", "libSceErrorDialog", 0);
    
    // libSceNpProfileDialog
    module->addSymbolStub("Lg+NCE6pTwQ", "sceNpProfileDialogInitialize", "libSceNpProfileDialog", "libSceNpProfileDialog");
    module->addSymbolStub("haVZE9FgKqE", "sceNpProfileDialogUpdateStatus", "libSceNpProfileDialog", "libSceNpProfileDialog", 3);
    module->addSymbolStub("8rhLl1-0W-o", "sceNpProfileDialogGetResult", "libSceNpProfileDialog", "libSceNpProfileDialog");
    module->addSymbolStub("0Sp9vJcB1-w", "sceNpProfileDialogTerminate", "libSceNpProfileDialog", "libSceNpProfileDialog");
    
    // libSceNpCommerce
    module->addSymbolStub("LR5cwFMMCVE", "sceNpCommerceDialogUpdateStatus", "libSceNpCommerce", "libSceNpCommerce", 3);
    module->addSymbolStub("r42bWcQbtZY", "sceNpCommerceDialogGetResult", "libSceNpCommerce", "libSceNpCommerce");
    module->addSymbolStub("m-I92Ab50W8", "sceNpCommerceDialogTerminate", "libSceNpCommerce", "libSceNpCommerce");
    
    // libSceInvitationDialog
    module->addSymbolStub("9+g9iOq+7kg", "sceInvitationDialogUpdateStatus", "libSceInvitationDialog", "libSceInvitationDialog", 0);
    
    // libSceImeDialog
    module->addSymbolStub("IADmD4tScBY", "sceImeDialogGetStatus", "libSceImeDialog", "libSceImeDialog", 0);
    
    // libSceWebBrowserDialog
    module->addSymbolStub("h1dR-t5ISgg", "sceWebBrowserDialogUpdateStatus", "libSceWebBrowserDialog", "libSceWebBrowserDialog", 3);
    module->addSymbolStub("CFTG6a8TjOU", "sceWebBrowserDialogGetStatus", "libSceWebBrowserDialog", "libSceWebBrowserDialog", 0);
    
    // libSceImeBackend
    module->addSymbolStub("9GIB91cj1wc", "sceImeBackendParamInit", "libSceImeBackend", "libSceImeBackend");
    module->addSymbolStub("5yb0O2TsYvw", "sceImeBackendOpen", "libSceImeBackend", "libSceImeBackend");
    module->addSymbolStub("w99gsSH-nWA", "sceImeBackendGetStatus", "libSceImeBackend", "libSceImeBackend");
    module->addSymbolStub("Xva83fZZ7D4", "sceImeBackendGetCaretIndex", "libSceImeBackend", "libSceImeBackend");
    module->addSymbolStub("co7QF5zVjnY", "sceImeBackendGetConvertString", "libSceImeBackend", "libSceImeBackend");
    module->addSymbolStub("BfYmnUyy0ew", "sceImeBackendClose", "libSceImeBackend", "libSceImeBackend");
    
    // libSceNpAuth
    module->addSymbolStub("N+mr7GjTvr8", "sceNpAuthCreateAsyncRequest", "libSceNpAuth", "libSceNpAuth", 1);
    module->addSymbolStub("KxGkOrQJTqY", "sceNpAuthGetAuthorizationCode", "libSceNpAuth", "libSceNpAuth");   // TODO: At least store a dummy value in auth_code ptr
    module->addSymbolStub("gjSyfzSsDcE", "sceNpAuthPollAsync", "libSceNpAuth", "libSceNpAuth");              // TODO: At least store a dummy value in result ptr
    module->addSymbolStub("H8wG9Bk-nPc", "sceNpAuthDeleteRequest", "libSceNpAuth", "libSceNpAuth");
    
    // libSceNpSignaling
    module->addSymbolStub("3KOuC4RmZZU", "sceNpSignalingInitialize", "libSceNpSignaling", "libSceNpSignaling");
    
    // libSceNetCtl
    module->addSymbolExport("uBPlr0lbuiI", "sceNetCtlGetState", "libSceNetCtl", "libSceNetCtl", (void*)&sceNetCtlGetState);
    module->addSymbolStub("gky0+oaNM4k", "sceNetCtlInit", "libSceNetCtl", "libSceNetCtl");
    module->addSymbolStub("obuxdTiwkF8", "sceNetCtlGetInfo", "libSceNetCtl", "libSceNetCtl");
    module->addSymbolStub("UJ+Z7Q+4ck0", "sceNetCtlRegisterCallback", "libSceNetCtl", "libSceNetCtl");   // Should store callback id
    module->addSymbolStub("Rqm2OnZMCz0", "sceNetCtlUnregisterCallback", "libSceNetCtl", "libSceNetCtl");
    module->addSymbolStub("iQw3iQPhvUQ", "sceNetCtlCheckCallback", "libSceNetCtl", "libSceNetCtl");   // Should store callback id
    module->addSymbolStub("JO4yuTuMoKI", "sceNetCtlGetNatInfo", "libSceNetCtl", "libSceNetCtl");
    module->addSymbolStub("Z4wwCFiBELQ", "sceNetCtlTerm", "libSceNetCtl", "libSceNetCtl");
    
    // libSceNpTus
    module->addSymbolStub("BIkMmUfNKWM", "sceNpTusCreateNpTitleCtx", "libSceNpTus", "libSceNpTus", 1);
    module->addSymbolStub("3bh2aBvvmvM", "sceNpTusCreateRequest", "libSceNpTus", "libSceNpTus", 1);
    module->addSymbolStub("XOzszO4ONWU", "sceNpTusGetData", "libSceNpTus", "libSceNpTus");
    module->addSymbolStub("CcIH40dYS88", "sceNpTusDeleteRequest", "libSceNpTus", "libSceNpTus");

    module->addSymbolStub("wIsKy+TfeLs", "sceNetCtlRegisterCallbackForNpToolkit", "libSceNetCtlForNpToolkit", "libSceNetCtl");
    module->addSymbolStub("u5oqtlIP+Fw", "sceNetCtlCheckCallbackForNpToolkit", "libSceNetCtlForNpToolkit", "libSceNetCtl");

    // libSceSsl
    module->addSymbolStub("hdpVEUDFW3s", "sceSslInit", "libSceSsl", "libSceSsl", 1);
    module->addSymbolExport("TDfQqO-gMbY", "sceSslGetCaCerts", "libSceSsl", "libSceSsl", (void*)&sceSslGetCaCerts);
    module->addSymbolStub("qIvLs0gYxi0", "sceSslFreeCaCerts", "libSceSsl", "libSceSsl");
    module->addSymbolStub("0K1yQ6Lv-Yc", "sceSslTerm", "libSceSsl", "libSceSsl");
    
    // libSceHttp
    module->addSymbolStub("A9cVMUtEp4Y", "sceHttpInit", "libSceHttp", "libSceHttp", 1);
    module->addSymbolStub("Kiwv9r4IZCc", "sceHttpCreateConnection", "libSceHttp", "libSceHttp");
    module->addSymbolStub("Aeu5wVKkF9w", "sceHttpCreateRequestWithURL", "libSceHttp", "libSceHttp");
    module->addSymbolStub("0gYjPTR-6cY", "sceHttpCreateTemplate", "libSceHttp", "libSceHttp", 1);
    module->addSymbolStub("htyBOoWeS58", "sceHttpsSetSslCallback", "libSceHttp", "libSceHttp");
    module->addSymbolStub("s2-NPIvz+iA", "sceHttpSetNonblock", "libSceHttp", "libSceHttp");
    module->addSymbolStub("6381dWF+xsQ", "sceHttpCreateEpoll", "libSceHttp", "libSceHttp");
    module->addSymbolStub("qgxDBjorUxs", "sceHttpCreateConnectionWithURL", "libSceHttp", "libSceHttp");
    module->addSymbolStub("Cnp77podkCU", "sceHttpCreateRequestWithURL2", "libSceHttp", "libSceHttp");
    module->addSymbolStub("EY28T2bkN7k", "sceHttpAddRequestHeader", "libSceHttp", "libSceHttp");
    module->addSymbolStub("-xm7kZQNpHI", "sceHttpSetEpoll", "libSceHttp", "libSceHttp");
    module->addSymbolStub("T-mGo9f3Pu4", "sceHttpSetAutoRedirect", "libSceHttp", "libSceHttp");
    module->addSymbolStub("qFg2SuyTJJY", "sceHttpSetAuthEnabled", "libSceHttp", "libSceHttp");
    module->addSymbolStub("1e2BNwI-XzE", "sceHttpSendRequest", "libSceHttp", "libSceHttp");
    module->addSymbolStub("0a2TBNfE3BU", "sceHttpGetStatusCode", "libSceHttp", "libSceHttp");
    module->addSymbolStub("aCYPMSUIaP8", "sceHttpGetAllResponseHeaders", "libSceHttp", "libSceHttp");
    module->addSymbolStub("yuO2H2Uvnos", "sceHttpGetResponseContentLength", "libSceHttp", "libSceHttp");
    module->addSymbolStub("P5pdoykPYTk", "sceHttpReadData", "libSceHttp", "libSceHttp");
    module->addSymbolStub("qISjDHrxONc", "sceHttpWaitRequest", "libSceHttp", "libSceHttp");
    module->addSymbolStub("4I8vEpuEhZ8", "sceHttpDeleteTemplate", "libSceHttp", "libSceHttp");
    module->addSymbolStub("Ik-KpLTlf7Q", "sceHttpTerm", "libSceHttp", "libSceHttp");
    
    // libSceNpCommon
    module->addSymbolStub("i8UmXTSq7N4", "sceNpCmpNpId", "libSceNpCommon", "libSceNpCommon");
    
    // libSceNpWebApi
    module->addSymbolStub("G3AnLNdRBjE", "sceNpWebApiInitialize", "libSceNpWebApi", "libSceNpWebApi", 1);
    module->addSymbolStub("79M-JqvvGo0", "sceNpWebApiCreateHandle", "libSceNpWebApi", "libSceNpWebApi", 1);
    module->addSymbolStub("y5Ta5JCzQHY", "sceNpWebApiCreatePushEventFilter", "libSceNpWebApi", "libSceNpWebApi");
    module->addSymbolStub("rdgs5Z1MyFw", "sceNpWebApiCreateRequest", "libSceNpWebApi", "libSceNpWebApi");
    module->addSymbolStub("KjNeZ-29ysQ", "sceNpWebApiSendRequest2", "libSceNpWebApi", "libSceNpWebApi");
    module->addSymbolStub("PfSTDCgNMgc", "sceNpWebApiRegisterPushEventCallback", "libSceNpWebApi", "libSceNpWebApi");
    
    // libSceAudioIn
    module->addSymbolStub("5NE8Sjc7VC8", "sceAudioInOpen", "libSceAudioIn", "libSceAudioIn", 1);
    module->addSymbolExport("LozEOU8+anM", "sceAudioInInput", "libSceAudioIn", "libSceAudioIn", (void*)&sceAudioInInput);
    
    // libSceVoice
    module->addSymbolStub("9TrhuGzberQ", "sceVoiceInit", "libSceVoice", "libSceVoice");
    module->addSymbolStub("nXpje5yNpaE", "sceVoiceCreatePort", "libSceVoice", "libSceVoice");
    module->addSymbolStub("54phPH2LZls", "sceVoiceStart", "libSceVoice", "libSceVoice");
    module->addSymbolStub("CrLqDwWLoXM", "sceVoiceGetPortInfo", "libSceVoice", "libSceVoice");
    module->addSymbolStub("oV9GAdJ23Gw", "sceVoiceConnectIPortToOPort", "libSceVoice", "libSceVoice");
    module->addSymbolStub("elcxZTEfHZM", "sceVoiceGetPortAttr", "libSceVoice", "libSceVoice");
    module->addSymbolStub("QBFoAIjJoXQ", "sceVoiceSetVolume", "libSceVoice", "libSceVoice");

    // libSceDiscMap
    module->addSymbolStub("lbQKqsERhtE", "sceDiscMapIsRequestOnHDD", "libSceDiscMap", "libSceDiscMap", 0x81100004);
    module->addSymbolStub("ioKMruft1ek", "sceDiscMap_ioKMruft1ek", "libSceDiscMap", "libSceDiscMap");
    module->addSymbolStub("fJgP+wqifno", "sceDiscMap_fJgP+wqifno", "libSceDiscMap", "libSceDiscMap");
    
    // libSceGameLiveStreaming
    module->addSymbolStub("kvYEw2lBndk", "sceGameLiveStreamingInitialize", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("q-kxuaF7URU", "sceGameLiveStreamingSetMaxBitrate", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("K0QxEbD7q+c", "sceGameLiveStreamingPermitLiveStreaming", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("dWM80AX39o4", "sceGameLiveStreamingEnableLiveStreaming", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("-EHnU68gExU", "sceGameLiveStreamingPermitServerSideRecording", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("wBOQWjbWMfU", "sceGameLiveStreamingEnableSocialFeedback", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("ycodiP2I0xo", "sceGameLiveStreamingSetPresetSocialFeedbackCommands", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("yeQKjHETi40", "sceGameLiveStreamingGetSocialFeedbackMessagesCount", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module->addSymbolStub("CoPMx369EqM", "sceGameLiveStreamingGetCurrentStatus", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    
    // libSceCamera
    module->addSymbolStub("p6n3Npi3YY4", "sceCameraIsAttached", "libSceCamera", "libSceCamera");
    
    // libSceMove
    module->addSymbolStub("j1ITE-EoJmE", "sceMoveInit", "libSceMove", "libSceMove");
    module->addSymbolStub("HzC60MfjJxU", "sceMoveOpen", "libSceMove", "libSceMove", 1);
    module->addSymbolStub("f2bcpK6kJfg", "sceMoveReadStateRecent", "libSceMove", "libSceMove");
    
    // libSceMouse
    module->addSymbolStub("wadT3QBCGY0", "sceKeyboardInit", "libSceKeyboard", "libSceKeyboard");
    module->addSymbolStub("HJ+KnEHcaxI", "sceKeyboardOpen", "libSceKeyboard", "libSceKeyboard", 1);
    module->addSymbolStub("xybbGMCr738", "sceKeyboardRead", "libSceKeyboard", "libSceKeyboard");
    module->addSymbolStub("6HpE68bzX6M", "sceKeyboardReadState", "libSceKeyboard", "libSceKeyboard");
    
    // libSceMouse
    module->addSymbolStub("Qs0wWulgl7U", "sceMouseInit", "libSceMouse", "libSceMouse");
    module->addSymbolStub("RaqxZIf6DvE", "sceMouseOpen", "libSceMouse", "libSceMouse", 1);
    module->addSymbolStub("x8qnXqh-tiM", "sceMouseRead", "libSceMouse", "libSceMouse");

    // libSceMoveTracker
    module->addSymbolStub("F4w2atwG428", "sceMoveTrackerInit", "libSceMoveTracker", "libSceMoveTracker");
    module->addSymbolStub("gg1d4KsyVVs", "sceMoveTrackerGetWorkingMemorySize", "libSceMoveTracker", "libSceMoveTracker");
    module->addSymbolStub("-Y8hlMgBsr4", "sceMoveTrackerControllersUpdate", "libSceMoveTracker", "libSceMoveTracker");
    module->addSymbolStub("YV2CtE7qX8M", "sceMoveTrackerGetState", "libSceMoveTracker", "libSceMoveTracker");
    
    // libSceVideoRecording
    module->addSymbolStub("Fc8qxlKINYQ", "sceVideoRecordingSetInfo", "libSceVideoRecording", "libSceVideoRecording");
    
    // libSceRemoteplay
    module->addSymbolStub("k1SwgkMSOM8", "sceRemoteplayInitialize", "libSceRemoteplay", "libSceRemoteplay");
    module->addSymbolStub("xQeIryTX7dY", "sceRemoteplayApprove", "libSceRemoteplay", "libSceRemoteplay");
    module->addSymbolStub("g3PNjYKWqnQ", "sceRemoteplayGetConnectionStatus", "libSceRemoteplay", "libSceRemoteplay");
    
    // libSceIme
    module->addSymbolStub("eaFXjfJv3xs", "sceImeKeyboardOpen", "libSceIme", "libSceIme");
    module->addSymbolStub("-4GCfYdNF1s", "sceImeUpdate", "libSceIme", "libSceIme");
    module->addSymbolStub("VkqLPArfFdc", "sceImeKeyboardGetInfo", "libSceIme", "libSceIme");
    module->addSymbolStub("dKadqZFgKKQ", "sceImeKeyboardGetResourceId", "libSceIme", "libSceIme");
    
    // libSceAvPlayer
    module->addSymbolStub("aS66RI0gGgo", "sceAvPlayerInit", "libSceAvPlayer", "libSceAvPlayer");
    
    // libSceRudp
    module->addSymbolStub("amuBfI-AQc4", "sceRudpInit", "libSceRudp", "libSceRudp");
    module->addSymbolStub("6PBNpsgyaxw", "sceRudpEnableInternalIOThread", "libSceRudp", "libSceRudp");
    
    // libSceCompanionUtil
    module->addSymbolStub("xb1xlIhf0QY", "sceCompanionUtilInitialize", "libSceCompanionUtil", "libSceCompanionUtil");
    module->addSymbolStub("IPN-FRSrafk", "sceCompanionUtilOptParamInitialize", "libSceCompanionUtil", "libSceCompanionUtil");
    module->addSymbolStub("cE5Msy11WhU", "sceCompanionUtilGetEvent", "libSceCompanionUtil", "libSceCompanionUtil", 0x80AD0008 /* No event */);
    
    // libSceRazorCpu
    module->addSymbolStub("PAytDtFGpqY", "sceRazorCpuFiberSwitch", "libSceRazorCpu", "libSceRazorCpu");
    module->addSymbolStub("G90IIOtgFQ0", "sceRazorCpuFiberLogNameChange", "libSceRazorCpu", "libSceRazorCpu");
    
    // libSceCoredump
    module->addSymbolStub("8zLSfEfW5AU", "sceCoredumpRegisterCoredumpHandler", "libSceCoredump", "libkernel");
    
    // ulobjmgr (TODO: What is this?)
    module->addSymbolStub("SweJO7t3pkk", "ulobjmgr_SweJO7t3pkk", "ulobjmgr", "ulobjmgr");
    module->addSymbolStub("BG26hBGiNlw", "ulobjmgr_BG26hBGiNlw", "ulobjmgr", "ulobjmgr");
    module->addSymbolStub("Smf+fUNblPc", "ulobjmgr_Smf+fUNblPc", "ulobjmgr", "ulobjmgr");
    
    // dlcldr (TODO: What is this?)
    module->addSymbolStub("4qL3yyKEXoM", "dlcldr_4qL3yyKEXoM", "dlcldr", "dlcldr");

    // libSceAvPlayer
    module->addSymbolStub("KMcEa+rHsIo", "sceAvPlayerAddSource", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("x8uvuFOPZhU", "sceAvPlayerAddSourceEx", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("buMCiJftcfw", "sceAvPlayerChangeStream", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("NkJwDzKmIlw", "sceAvPlayerClose", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("wwM99gjFf1Y", "sceAvPlayerCurrentTime", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("BOVKAzRmuTQ", "sceAvPlayerDisableStream", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("ODJK2sn9w4A", "sceAvPlayerEnableStream", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("Wnp1OVcrZgk", "sceAvPlayerGetAudioData", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("d8FcbzfAdQw", "sceAvPlayerGetStreamInfo", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("ctTAcF5DiKQ", "sceAvPlayerGetStreamInfoEx", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("o3+RWnHViSg", "sceAvPlayerGetVideoData", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("JdksQu8pNdQ", "sceAvPlayerGetVideoDataEx", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("aS66RI0gGgo", "sceAvPlayerInit", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("o9eWRkSL+M4", "sceAvPlayerInitEx", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("UbQoYawOsfY", "sceAvPlayerIsActive", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("XC9wM+xULz8", "sceAvPlayerJumpToTime", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("9y5v+fGN4Wk", "sceAvPlayerPause", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("HD1YKVU26-M", "sceAvPlayerPostInit", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("agig-iDRrTE", "sceAvPlayerPrintf", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("w5moABNwnRY", "sceAvPlayerResume", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("N6Oy-EjduiY", "sceAvPlayerSetAvailableBandwidth", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("k-q+xOxdc3E", "sceAvPlayerSetAvSyncMode", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("eBTreZ84JFY", "sceAvPlayerSetLogCallback", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("OVths0xGfho", "sceAvPlayerSetLooping", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("av8Z++94rs0", "sceAvPlayerSetTrickSpeed", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("ET4Gr-Uu07s", "sceAvPlayerStart", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("NxSdL9t-KXk", "sceAvPlayerStartEx", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("ZC17w3vB5Lo", "sceAvPlayerStop", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("hdTyRzCXQeQ", "sceAvPlayerStreamCount", "libSceAvPlayer", "libSceAvPlayer");
    module->addSymbolStub("yN7Jhuv8g24", "sceAvPlayerVprintf", "libSceAvPlayer", "libSceAvPlayer");

    return module;
}

}   // End namespace PS4::OS::HLE