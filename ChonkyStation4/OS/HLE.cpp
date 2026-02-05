#include "HLE.hpp"
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>
#include <OS/Libraries/SceSystemService/SceSystemService.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Libraries/SceNpManager/SceNpManager.hpp>
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


// Stub until we implement audio input
s32 PS4_FUNC sceAudioInInput(s32 handle, void* ptr) {
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}

s32 PS4_FUNC sceNetCtlGetState(s32* state) {
    //printf("sceNetCtlGetState()\n");
    *state = 0; // Disconnected
    return SCE_OK;
}

namespace PS4::OS::HLE {

// Create a dummy HLE module that only contains the symbol exports for HLE functions
Module buildHLEModule() {
    Module module;
    module.filename = "HLE";

    PS4::OS::Libs::Kernel::init(module);
    PS4::OS::Libs::SceVideoOut::init(module);
    PS4::OS::Libs::SceGnmDriver::init(module);
    PS4::OS::Libs::SceSystemService::init(module);
    PS4::OS::Libs::SceUserService::init(module);
    PS4::OS::Libs::SceNpManager::init(module);
    PS4::OS::Libs::SceSysmodule::init(module);
    PS4::OS::Libs::SceSaveData::init(module);
    PS4::OS::Libs::SceSaveDataDialog::init(module);
    PS4::OS::Libs::SceNpTrophy::init(module);
    PS4::OS::Libs::ScePad::init(module);
    PS4::OS::Libs::SceAudioOut::init(module);
    PS4::OS::Libs::ScePlayGo::init(module);
    PS4::OS::Libs::SceRtc::init(module);
    PS4::OS::Libs::SceNet::init(module);
    PS4::OS::Libs::SceRandom::init(module);

    // libSceAppContent
    module.addSymbolStub("R9lA82OraNs", "sceAppContentInitialize", "libSceAppContent", "libSceAppContentUtil");
    module.addSymbolStub("xnd8BJzAxmk", "sceAppContentGetAddcontInfoList", "libSceAppContent", "libSceAppContentUtil");
    module.addSymbolStub("99b82IKXpH4", "sceAppContentAppParamGetInt", "libSceAppContent", "libSceAppContentUtil"); // TODO: Important

    // libSceScreenShot
    module.addSymbolStub("2xxUtuC-RzE", "sceScreenShotEnable", "libSceScreenShot", "libSceScreenShot");
    module.addSymbolStub("73WQ4Jj0nJI", "sceScreenShotSetOverlayImageWithOrigin", "libSceScreenShot", "libSceScreenShot");
    module.addSymbolStub("G7KlmIYFIZc", "sceScreenShotSetParam", "libSceScreenShot", "libSceScreenShot");
    
    // libSceMsgDialog
    module.addSymbolStub("lDqxaY1UbEo", "sceMsgDialogInitialize", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("b06Hh0DPEaE", "sceMsgDialogOpen", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("6fIC3XKt2k0", "sceMsgDialogUpdateStatus", "libSceMsgDialog", "libSceMsgDialog", 3);
    module.addSymbolStub("CWVW78Qc3fI", "sceMsgDialogGetStatus", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("Lr8ovHH9l6A", "sceMsgDialogGetResult", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("HTrcDKlFKuM", "sceMsgDialogClose", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("ePw-kqZmelo", "sceMsgDialogTerminate", "libSceMsgDialog", "libSceMsgDialog");

    // libSceCommonDialog
    module.addSymbolStub("uoUpLGNkygk", "sceCommonDialogInitialize", "libSceCommonDialog", "libSceCommonDialog");
    
    // libSceNpScore
    module.addSymbolStub("KnNA1TEgtBI", "sceNpScoreCreateNpTitleCtx", "libSceNpScore", "libSceNpScore", 1);
    module.addSymbolStub("GWnWQNXZH5M", "sceNpScoreCreateNpTitleCtxA", "libSceNpScore", "libSceNpScore", 1);
    module.addSymbolStub("gW8qyjYrUbk", "sceNpScoreCreateRequest", "libSceNpScore", "libSceNpScore", 1);
    module.addSymbolStub("ANJssPz3mY0", "sceNpScoreRecordScoreAsync", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("8kuIzUw6utQ", "sceNpScoreGetFriendsRanking", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("9mZEgoiEq6Y", "sceNpScoreGetRankingByNpId", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("dK8-SgYf6r4", "sceNpScoreDeleteRequest", "libSceNpScore", "libSceNpScore", 0);
    module.addSymbolStub("m1DfNRstkSQ", "sceNpScorePollAsync", "libSceNpScore", "libSceNpScore", 0);
    
    // libSceNpMatching2
    module.addSymbolStub("10t3e5+JPnU", "sceNpMatching2Initialize", "libSceNpMatching2", "libSceNpMatching2");
    module.addSymbolStub("fQQfP87I7hs", "sceNpMatching2RegisterContextCallback", "libSceNpMatching2", "libSceNpMatching2");
    
    // libSceNpParty
    module.addSymbolStub("lhYCTQmBkds", "sceNpPartyInitialize", "libSceNpParty", "libSceNpParty");
    module.addSymbolStub("kA88gbv71ao", "sceNpPartyRegisterHandler", "libSceNpParty", "libSceNpParty");
    module.addSymbolStub("3e4k2mzLkmc", "sceNpPartyCheckCallback", "libSceNpParty", "libSceNpParty");
    
    // libSceErrorDialog
    module.addSymbolStub("I88KChlynSs", "sceErrorDialogInitialize", "libSceErrorDialog", "libSceErrorDialog", 0);
    module.addSymbolStub("M2ZF-ClLhgY", "sceErrorDialogOpen", "libSceErrorDialog", "libSceErrorDialog", 0);
    module.addSymbolStub("WWiGuh9XfgQ", "sceErrorDialogUpdateStatus", "libSceErrorDialog", "libSceErrorDialog", 0);
    
    // libSceNpProfileDialog
    module.addSymbolStub("Lg+NCE6pTwQ", "sceNpProfileDialogInitialize", "libSceNpProfileDialog", "libSceNpProfileDialog", 0);
    
    // libSceNpAuth
    module.addSymbolStub("N+mr7GjTvr8", "sceNpAuthCreateAsyncRequest", "libSceNpAuth", "libSceNpAuth", 1);
    module.addSymbolStub("KxGkOrQJTqY", "sceNpAuthGetAuthorizationCode", "libSceNpAuth", "libSceNpAuth");   // TODO: At least store a dummy value in auth_code ptr
    module.addSymbolStub("gjSyfzSsDcE", "sceNpAuthPollAsync", "libSceNpAuth", "libSceNpAuth");              // TODO: At least store a dummy value in result ptr
    module.addSymbolStub("H8wG9Bk-nPc", "sceNpAuthDeleteRequest", "libSceNpAuth", "libSceNpAuth");
    
    // libSceNpSignaling
    module.addSymbolStub("3KOuC4RmZZU", "sceNpSignalingInitialize", "libSceNpSignaling", "libSceNpSignaling");
    
    // libSceNetCtl
    module.addSymbolExport("uBPlr0lbuiI", "sceNetCtlGetState", "libSceNetCtl", "libSceNetCtl", (void*)&sceNetCtlGetState);
    module.addSymbolStub("gky0+oaNM4k", "sceNetCtlInit", "libSceNetCtl", "libSceNetCtl");
    module.addSymbolStub("obuxdTiwkF8", "sceNetCtlGetInfo", "libSceNetCtl", "libSceNetCtl");
    module.addSymbolStub("UJ+Z7Q+4ck0", "sceNetCtlRegisterCallback", "libSceNetCtl", "libSceNetCtl");   // Should store callback id
    module.addSymbolStub("Rqm2OnZMCz0", "sceNetCtlUnregisterCallback", "libSceNetCtl", "libSceNetCtl");
    module.addSymbolStub("iQw3iQPhvUQ", "sceNetCtlCheckCallback", "libSceNetCtl", "libSceNetCtl");   // Should store callback id
    module.addSymbolStub("Z4wwCFiBELQ", "sceNetCtlTerm", "libSceNetCtl", "libSceNetCtl");

    // libSceSsl
    module.addSymbolStub("hdpVEUDFW3s", "sceSslInit", "libSceSsl", "libSceSsl", 1);
    module.addSymbolStub("0K1yQ6Lv-Yc", "sceSslTerm", "libSceSsl", "libSceSsl", 1);
    
    // libSceHttp
    module.addSymbolStub("A9cVMUtEp4Y", "sceHttpInit", "libSceHttp", "libSceHttp", 1);
    module.addSymbolStub("0gYjPTR-6cY", "sceHttpCreateTemplate", "libSceHttp", "libSceHttp", 1);
    module.addSymbolStub("htyBOoWeS58", "sceHttpsSetSslCallback", "libSceHttp", "libSceHttp");
    module.addSymbolStub("Ik-KpLTlf7Q", "sceHttpTerm", "libSceHttp", "libSceHttp");
    
    // libSceNpWebApi
    module.addSymbolStub("G3AnLNdRBjE", "sceNpWebApiInitialize", "libSceNpWebApi", "libSceNpWebApi", 1);
    module.addSymbolStub("y5Ta5JCzQHY", "sceNpWebApiCreatePushEventFilter", "libSceNpWebApi", "libSceNpWebApi");
    module.addSymbolStub("PfSTDCgNMgc", "sceNpWebApiRegisterPushEventCallback", "libSceNpWebApi", "libSceNpWebApi");
    
    // libSceAudioIn
    module.addSymbolStub("5NE8Sjc7VC8", "sceAudioInOpen", "libSceAudioIn", "libSceAudioIn", 1);
    module.addSymbolExport("LozEOU8+anM", "sceAudioInInput", "libSceAudioIn", "libSceAudioIn", (void*)&sceAudioInInput);

    // libSceDiscMap
    module.addSymbolStub("lbQKqsERhtE", "sceDiscMapIsRequestOnHDD", "libSceDiscMap", "libSceDiscMap", 0x81100004);
    module.addSymbolStub("ioKMruft1ek", "sceDiscMap_ioKMruft1ek", "libSceDiscMap", "libSceDiscMap");
    module.addSymbolStub("fJgP+wqifno", "sceDiscMap_fJgP+wqifno", "libSceDiscMap", "libSceDiscMap");
    
    // libSceGameLiveStreaming
    module.addSymbolStub("kvYEw2lBndk", "sceGameLiveStreamingInitialize", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module.addSymbolStub("q-kxuaF7URU", "sceGameLiveStreamingSetMaxBitrate", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module.addSymbolStub("K0QxEbD7q+c", "sceGameLiveStreamingPermitLiveStreaming", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module.addSymbolStub("dWM80AX39o4", "sceGameLiveStreamingEnableLiveStreaming", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module.addSymbolStub("-EHnU68gExU", "sceGameLiveStreamingPermitServerSideRecording", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module.addSymbolStub("wBOQWjbWMfU", "sceGameLiveStreamingEnableSocialFeedback", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    module.addSymbolStub("ycodiP2I0xo", "sceGameLiveStreamingSetPresetSocialFeedbackCommands", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    
    // libSceCamera
    module.addSymbolStub("p6n3Npi3YY4", "sceCameraIsAttached", "libSceCamera", "libSceCamera");
    
    // libSceMove
    module.addSymbolStub("j1ITE-EoJmE", "sceMoveInit", "libSceMove", "libSceMove");
    module.addSymbolStub("HzC60MfjJxU", "sceMoveOpen", "libSceMove", "libSceMove", 1);
    module.addSymbolStub("f2bcpK6kJfg", "sceMoveReadStateRecent", "libSceMove", "libSceMove");

    // libSceMoveTracker
    module.addSymbolStub("F4w2atwG428", "sceMoveTrackerInit", "libSceMoveTracker", "libSceMoveTracker");
    module.addSymbolStub("gg1d4KsyVVs", "sceMoveTrackerGetWorkingMemorySize", "libSceMoveTracker", "libSceMoveTracker");
    module.addSymbolStub("-Y8hlMgBsr4", "sceMoveTrackerControllersUpdate", "libSceMoveTracker", "libSceMoveTracker");
    module.addSymbolStub("YV2CtE7qX8M", "sceMoveTrackerGetState", "libSceMoveTracker", "libSceMoveTracker");
    
    // libSceVideoRecording
    module.addSymbolStub("Fc8qxlKINYQ", "sceVideoRecordingSetInfo", "libSceVideoRecording", "libSceVideoRecording");
    
    // libSceRemoteplay
    module.addSymbolStub("k1SwgkMSOM8", "sceRemoteplayInitialize", "libSceRemoteplay", "libSceRemoteplay");
    module.addSymbolStub("xQeIryTX7dY", "sceRemoteplayApprove", "libSceRemoteplay", "libSceRemoteplay");
    module.addSymbolStub("g3PNjYKWqnQ", "sceRemoteplayGetConnectionStatus", "libSceRemoteplay", "libSceRemoteplay");
    
    // libSceAjm
    module.addSymbolStub("dl+4eHSzUu4", "sceAjmInitialize", "libSceAjm", "libSceAjm");
    module.addSymbolStub("Q3dyFuwGn64", "sceAjmModuleRegister", "libSceAjm", "libSceAjm");
    module.addSymbolStub("AxoDrINp4J8", "sceAjmInstanceCreate", "libSceAjm", "libSceAjm");
    module.addSymbolStub("dmDybN--Fn8", "sceAjmBatchJobControlBufferRa", "libSceAjm", "libSceAjm");
    module.addSymbolStub("7jdAXK+2fMo", "sceAjmBatchJobRunSplitBufferRa", "libSceAjm", "libSceAjm");
    module.addSymbolStub("fFFkk0xfGWs", "sceAjmBatchStartBuffer", "libSceAjm", "libSceAjm");
    module.addSymbolStub("-qLsfDAywIY", "sceAjmBatchWait", "libSceAjm", "libSceAjm");
    
    // libSceAvPlayer
    module.addSymbolStub("aS66RI0gGgo", "sceAvPlayerInit", "libSceAvPlayer", "libSceAvPlayer");
    
    // ulobjmgr (TODO: What is this?)
    module.addSymbolStub("SweJO7t3pkk", "ulobjmgr_SweJO7t3pkk", "ulobjmgr", "ulobjmgr");
    
    // dlcldr (TODO: What is this?)
    module.addSymbolStub("4qL3yyKEXoM", "dlcldr_4qL3yyKEXoM", "dlcldr", "dlcldr");

    return module;
}

}   // End namespace PS4::OS::HLE