#include "HLE.hpp"
#include <Configuration.hpp>
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>
#include <OS/Libraries/SceSystemService/SceSystemService.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Libraries/SceNpManager/SceNpManager.hpp>
#include <OS/Libraries/SceNpMatching/SceNpMatching.hpp>
#include <OS/Libraries/SceNpScore/SceNpScore.hpp>
#include <OS/Libraries/SceNpWebApi/SceNpWebApi.hpp>
#include <OS/Libraries/SceSysmodule/SceSysmodule.hpp>
#include <OS/Libraries/SceSaveData/SceSaveData.hpp>
#include <OS/Libraries/SceSaveDataDialog/SceSaveDataDialog.hpp>
#include <OS/Libraries/SceNpTrophy/SceNpTrophy.hpp>
#include <OS/Libraries/ScePad/ScePad.hpp>
#include <OS/Libraries/SceAudioOut/SceAudioOut.hpp>
#include <OS/Libraries/SceAudio3d/SceAudio3d.hpp>
#include <OS/Libraries/ScePlayGo/ScePlayGo.hpp>
#include <OS/Libraries/SceRtc/SceRtc.hpp>
#include <OS/Libraries/SceNet/SceNet.hpp>
#include <OS/Libraries/SceRandom/SceRandom.hpp>
#include <OS/Libraries/SceVideodec/SceVideodec.hpp>
#include <OS/Libraries/SceAjm/SceAjm.hpp>
#include <OS/Libraries/SceAppContent/SceAppContent.hpp>
#include <OS/Libraries/SceZlib/SceZlib.hpp>
#include <OS/Libraries/SceRegMgr/SceRegMgr.hpp>
#include <OS/Libraries/SceComposite/SceComposite.hpp>
#include <OS/Libraries/SceMbus/SceMbus.hpp>
#include <GCN/GCN.hpp>


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
    const std::string dummy_data = "chonky";
    certs->cert = new SceSslData();
    certs->cert[0].ptr = new char[1_KB];
    std::strcpy(certs->cert[0].ptr, dummy_data.data());
    certs->cert[0].size = dummy_data.length();

    certs->n_certs = 1;
    certs->pool = nullptr;
    return SCE_OK;
}

s32 PS4_FUNC sceNpPartyGetState(s16* state) {
    printf("sceNpPartyGetState(state=*%p)\n", state);
    *state = 2;     // SCE_NP_PARTY_STATE_NOT_IN_PARTY
    return SCE_OK;
}

struct SceVoiceBasePortInfo {
    s32 port_type;
    s32 state;
    u32* edges;
    u32 n_bytes;
    u32 frame_size;
    u16 n_edges;
    u16 reserved;
};

s32 PS4_FUNC sceVoiceGetPortInfo(u32 port_id, SceVoiceBasePortInfo* port_info) {
    //printf("sceVoiceGetPortInfo(port_id=%d, port_info=*%p)\n", port_id, port_info);

    port_info->port_type = 0;
    port_info->state = 0;
    port_info->n_bytes = 0;
    port_info->frame_size = 1;  // Game will try to divide by this, can't be zero (happens in GTAV)
    port_info->n_edges = 0;
    return SCE_OK;
}

s32 PS4_FUNC sceKeyboardPadEmulateOpen() {
    //while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}

s32 PS4_FUNC sceVisionManagerGetWorkingMemorySize(u32* size1, u32* size2, u32* size3) {
    printf("sceVisionManagerGetWorkingMemorySize(size1=*%p, size2=*%p, size3=*%p)\n", size1, size2, size3);

    *size1 = 32_MB;
    *size2 = 32_MB;
    *size3 = 32_MB;
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
    PS4::OS::Libs::SceNpScore::init(*module);
    PS4::OS::Libs::SceSysmodule::init(*module);
    PS4::OS::Libs::SceSaveData::init(*module);
    PS4::OS::Libs::SceSaveDataDialog::init(*module);
    PS4::OS::Libs::SceNpTrophy::init(*module);
    PS4::OS::Libs::SceNpWebApi::init(*module);
    PS4::OS::Libs::ScePad::init(*module);
    PS4::OS::Libs::SceAudioOut::init(*module);
    PS4::OS::Libs::SceAudio3d::init(*module);
    PS4::OS::Libs::ScePlayGo::init(*module);
    //PS4::OS::Libs::SceRtc::init(*module);
    PS4::OS::Libs::SceNet::init(*module);
    PS4::OS::Libs::SceRandom::init(*module);
    PS4::OS::Libs::SceVideodec::init(*module);
    PS4::OS::Libs::SceAjm::init(*module);
    PS4::OS::Libs::SceAppContent::init(*module);
    PS4::OS::Libs::SceZlib::init(*module);
    PS4::OS::Libs::SceRegMgr::init(*module);
    PS4::OS::Libs::SceComposite::init(*module);
    PS4::OS::Libs::SceMbus::init(*module);

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
    module->addSymbolStub("+MCXJlWdi+s", "sceSharePlayGetCurrentConnectionInfoA", "libSceSharePlay", "libSceSharePlay");
    
    // libSceMsgDialog
    module->addSymbolStub("lDqxaY1UbEo", "sceMsgDialogInitialize", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("b06Hh0DPEaE", "sceMsgDialogOpen", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("6fIC3XKt2k0", "sceMsgDialogUpdateStatus", "libSceMsgDialog", "libSceMsgDialog", 3);
    module->addSymbolStub("CWVW78Qc3fI", "sceMsgDialogGetStatus", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("Lr8ovHH9l6A", "sceMsgDialogGetResult", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("Gc5k1qcK4fs", "sceMsgDialogProgressBarInc", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("6H-71OdrpXM", "sceMsgDialogProgressBarSetMsg", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("wTpfglkmv34", "sceMsgDialogProgressBarSetValue", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("HTrcDKlFKuM", "sceMsgDialogClose", "libSceMsgDialog", "libSceMsgDialog");
    module->addSymbolStub("ePw-kqZmelo", "sceMsgDialogTerminate", "libSceMsgDialog", "libSceMsgDialog");
    
    // libSceLoginDialog
    module->addSymbolStub("2rc+egSfb5A", "sceLoginDialogUpdateStatus", "libSceLoginDialog", "libSceLoginDialog", 3);
    module->addSymbolStub("Btkx21f1M8k", "sceLoginDialogGetResult", "libSceLoginDialog", "libSceLoginDialog");
    module->addSymbolStub("vMQJRUKsf3U", "sceLoginDialogTerminate", "libSceLoginDialog", "libSceLoginDialog");

    // libSceCommonDialog
    module->addSymbolStub("uoUpLGNkygk", "sceCommonDialogInitialize", "libSceCommonDialog", "libSceCommonDialog");
    module->addSymbolStub("BQ3tey0JmQM", "sceCommonDialogIsUsed", "libSceCommonDialog", "libSceCommonDialog", false);
    
    // libSceNpParty
    module->addSymbolExport("aEzKdJzATZ0", "sceNpPartyGetState", "libSceNpParty", "libSceNpParty", (void*)&sceNpPartyGetState);
    module->addSymbolStub("lhYCTQmBkds", "sceNpPartyInitialize", "libSceNpParty", "libSceNpParty");
    module->addSymbolStub("kA88gbv71ao", "sceNpPartyRegisterHandler", "libSceNpParty", "libSceNpParty");
    module->addSymbolStub("3e4k2mzLkmc", "sceNpPartyCheckCallback", "libSceNpParty", "libSceNpParty");
    module->addSymbolStub("+v4fVHMwFWc", "sceNpPartyRegisterHandlerA", "libSceNpParty", "libSceNpParty");
    module->addSymbolStub("T2UOKf00ZN0", "sceNpPartyGetMembers", "libSceNpParty", "libSceNpParty");
    
    // libSceErrorDialog
    module->addSymbolStub("I88KChlynSs", "sceErrorDialogInitialize", "libSceErrorDialog", "libSceErrorDialog", 0);
    module->addSymbolStub("M2ZF-ClLhgY", "sceErrorDialogOpen", "libSceErrorDialog", "libSceErrorDialog", 0);
    module->addSymbolStub("WWiGuh9XfgQ", "sceErrorDialogUpdateStatus", "libSceErrorDialog", "libSceErrorDialog", 0);
    module->addSymbolStub("9XAxK2PMwk8", "sceErrorDialogTerminate", "libSceErrorDialog", "libSceErrorDialog", 0);
    
    // libSceNpProfileDialog
    module->addSymbolStub("Lg+NCE6pTwQ", "sceNpProfileDialogInitialize", "libSceNpProfileDialog", "libSceNpProfileDialog");
    module->addSymbolStub("haVZE9FgKqE", "sceNpProfileDialogUpdateStatus", "libSceNpProfileDialog", "libSceNpProfileDialog", 3);
    module->addSymbolStub("8rhLl1-0W-o", "sceNpProfileDialogGetResult", "libSceNpProfileDialog", "libSceNpProfileDialog");
    module->addSymbolStub("0Sp9vJcB1-w", "sceNpProfileDialogTerminate", "libSceNpProfileDialog", "libSceNpProfileDialog");
    
    // libSceNpCommerce
    module->addSymbolStub("LR5cwFMMCVE", "sceNpCommerceDialogUpdateStatus", "libSceNpCommerce", "libSceNpCommerce", 0);
    module->addSymbolStub("r42bWcQbtZY", "sceNpCommerceDialogGetResult", "libSceNpCommerce", "libSceNpCommerce");
    module->addSymbolStub("m-I92Ab50W8", "sceNpCommerceDialogTerminate", "libSceNpCommerce", "libSceNpCommerce");
    
    // libSceInvitationDialog
    module->addSymbolStub("9+g9iOq+7kg", "sceInvitationDialogUpdateStatus", "libSceInvitationDialog", "libSceInvitationDialog", 0);
    
    // libSceImeDialog
    module->addSymbolStub("NUeBrN7hzf0", "sceImeDialogInit", "libSceImeDialog", "libSceImeDialog", 0);
    module->addSymbolStub("x01jxu+vxlc", "sceImeDialogGetResult", "libSceImeDialog", "libSceImeDialog", 0);
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
    module->addSymbolStub("qAUXQ9GdWp8", "sceNpAuthGetAuthorizationCodeA", "libSceNpAuth", "libSceNpAuth");   // TODO: At least store a dummy value in auth_code ptr
    module->addSymbolStub("gjSyfzSsDcE", "sceNpAuthPollAsync", "libSceNpAuth", "libSceNpAuth");              // TODO: At least store a dummy value in result ptr
    module->addSymbolStub("H8wG9Bk-nPc", "sceNpAuthDeleteRequest", "libSceNpAuth", "libSceNpAuth");
    
    // libSceNpSignaling
    module->addSymbolStub("3KOuC4RmZZU", "sceNpSignalingInitialize", "libSceNpSignaling", "libSceNpSignaling");
    
    // libSceNpTus
    module->addSymbolStub("BIkMmUfNKWM", "sceNpTusCreateNpTitleCtx", "libSceNpTus", "libSceNpTus", 1);
    module->addSymbolStub("3bh2aBvvmvM", "sceNpTusCreateRequest", "libSceNpTus", "libSceNpTus", 1);
    module->addSymbolStub("XOzszO4ONWU", "sceNpTusGetData", "libSceNpTus", "libSceNpTus");
    module->addSymbolStub("CcIH40dYS88", "sceNpTusDeleteRequest", "libSceNpTus", "libSceNpTus");

    module->addSymbolStub("wIsKy+TfeLs", "sceNetCtlRegisterCallbackForNpToolkit", "libSceNetCtlForNpToolkit", "libSceNetCtl");
    module->addSymbolStub("u5oqtlIP+Fw", "sceNetCtlCheckCallbackForNpToolkit", "libSceNetCtlForNpToolkit", "libSceNetCtl");

    // libSceSsl
    if (!Configuration::lle_ssl) {
        module->addSymbolExport("TDfQqO-gMbY", "sceSslGetCaCerts", "libSceSsl", "libSceSsl", (void*)&sceSslGetCaCerts);
        module->addSymbolStub("hdpVEUDFW3s", "sceSslInit", "libSceSsl", "libSceSsl", 1);
        module->addSymbolStub("P14ATpXc4J8", "sceSslCreateSslConnection", "libSceSsl", "libSceSsl");
        module->addSymbolStub("w1+L-27nYas", "sceSslDisableOptionInternalInsecure", "libSceSsl", "libSceSsl");
        module->addSymbolStub("g-zCwUKstEQ", "sceSslEnableOptionInternal", "libSceSsl", "libSceSsl");
        module->addSymbolStub("qIvLs0gYxi0", "sceSslFreeCaCerts", "libSceSsl", "libSceSsl");
        module->addSymbolStub("zXvd6iNyfgc", "sceSslConnect", "libSceSsl", "libSceSsl");
        module->addSymbolStub("p5bM5PPufFY", "sceSslSend", "libSceSsl", "libSceSsl");
        module->addSymbolStub("0K1yQ6Lv-Yc", "sceSslTerm", "libSceSsl", "libSceSsl");
    }

    // libSceHttp
    //module->addSymbolStub("A9cVMUtEp4Y", "sceHttpInit", "libSceHttp", "libSceHttp", 1);
    //module->addSymbolStub("Kiwv9r4IZCc", "sceHttpCreateConnection", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("Aeu5wVKkF9w", "sceHttpCreateRequestWithURL", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("0gYjPTR-6cY", "sceHttpCreateTemplate", "libSceHttp", "libSceHttp", 1);
    //module->addSymbolStub("f42K37mm5RM", "sceHttpsEnableOption", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("htyBOoWeS58", "sceHttpsSetSslCallback", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("s2-NPIvz+iA", "sceHttpSetNonblock", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("6381dWF+xsQ", "sceHttpCreateEpoll", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("qgxDBjorUxs", "sceHttpCreateConnectionWithURL", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("Cnp77podkCU", "sceHttpCreateRequestWithURL2", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("EY28T2bkN7k", "sceHttpAddRequestHeader", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("-xm7kZQNpHI", "sceHttpSetEpoll", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("T-mGo9f3Pu4", "sceHttpSetAutoRedirect", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("qFg2SuyTJJY", "sceHttpSetAuthEnabled", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("XNUoD2B9a6A", "sceHttpSetCookieEnabled", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("mSQCxzWTwVI", "sceHttpsDisableOption", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("1e2BNwI-XzE", "sceHttpSendRequest", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("0a2TBNfE3BU", "sceHttpGetStatusCode", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("aCYPMSUIaP8", "sceHttpGetAllResponseHeaders", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("yuO2H2Uvnos", "sceHttpGetResponseContentLength", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("P5pdoykPYTk", "sceHttpReadData", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("qISjDHrxONc", "sceHttpWaitRequest", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("hPTXo3bICzI", "sceHttpParseResponseHeader", "libSceHttp", "libSceHttp", 1);
    //module->addSymbolStub("4I8vEpuEhZ8", "sceHttpDeleteTemplate", "libSceHttp", "libSceHttp");
    //module->addSymbolStub("Ik-KpLTlf7Q", "sceHttpTerm", "libSceHttp", "libSceHttp");
    
    // libSceNpCommon
    //module->addSymbolStub("i8UmXTSq7N4", "sceNpCmpNpId", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("gMlY6eewr-c", "sceNpAllocateKernelMemoryWithAlignment", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("zNb6IxegrCE", "sceNpLwCondDestroy", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("++eqYdzB8Go", "sceNpLwCondInit", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("Xkn6VoN-wuQ", "sceNpLwCondSignal", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("FJ4DCt8VzVE", "sceNpLwCondSignalAll", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("Bwi+EP8VQ+g", "sceNpLwCondSignalTo", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("ExeLuE3EQCQ", "sceNpLwCondWait", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("4zxevggtYrQ", "sceNpLwMutexDestroy", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("1CiXI-MyEKs", "sceNpLwMutexInit", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("18j+qk6dRwk", "sceNpLwMutexLock", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("hp0kVgu5Fxw", "sceNpLwMutexTryLock", "libSceNpCommon", "libSceNpCommon");
    //module->addSymbolForPartialLLE("CQG2oyx1-nM", "sceNpLwMutexUnlock", "libSceNpCommon", "libSceNpCommon");

    // libSceNpUtility
    module->addSymbolStub("eYz4v5Uek9U", "sceNpLookupAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("JA4+sS39GMs", "sceNpLookupCreateAsyncRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("iQr9UxPHUFs", "sceNpLookupCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("8533Q+LU7EQ", "sceNpLookupCreateTitleCtx", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("vT9xhqPO6+0", "sceNpLookupCreateTitleCtxA", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("wLaxchvEEnk", "sceNpLookupDeleteRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("mtqDK9zkoIE", "sceNpLookupDeleteTitleCtx", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("1O96muPzhgU", "sceNpLookupNetAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("N0iF180VjGk", "sceNpLookupNetCensorComment", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("UI5t6Rx6s5I", "sceNpLookupNetConvertJidToNpId", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("ieROYX4vspk", "sceNpLookupNetConvertNpIdToJid", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("KUIRsku7EPk", "sceNpLookupNetCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("8DPEdJh9RkE", "sceNpLookupNetCreateTitleCtx", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("HL-venrRcnQ", "sceNpLookupNetDeleteRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("dxpUx7z9StY", "sceNpLookupNetDeleteTitleCtx", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("zVZE+fAhgFY", "sceNpLookupNetInit", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("DiUk6-mq--0", "sceNpLookupNetInitWithFunctionPointer", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("cpnwZeVIq8E", "sceNpLookupNetInitWithMemoryPool", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("ZXlTj9RRCFo", "sceNpLookupNetIsInit", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("2nEVmFiV6OI", "sceNpLookupNetNpId", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("jJH2P7KA4XU", "sceNpLookupNetSanitizeComment", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("NWtf77WCXJs", "sceNpLookupNetSetTimeout", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("Dbd5BY0QjG0", "sceNpLookupNetTerm", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("T6tnM1Uti4g", "sceNpLookupNpId", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("V4EVrruHuy8", "sceNpLookupPollAsync", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("0MV72WO7V34", "sceNpLookupSetTimeout", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("YX9dAus6baE", "sceNpLookupWaitAsync", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("Kq+ftR9LHlE", "sceNpServiceChecker2IntAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("si-TLaBGtdw", "sceNpServiceChecker2IntCheckServiceFlagArray", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("IG1Kd+k6U3s", "sceNpServiceChecker2IntCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("hBsBswrAiGM", "sceNpServiceChecker2IntDestroyRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("cvZrmlSlwn8", "sceNpServiceChecker2IntFinalize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("aUgLCb3pSOo", "sceNpServiceChecker2IntGetServiceAvailability", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("Yp2yK5YXb78", "sceNpServiceChecker2IntGetServiceAvailabilityA", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("05cqQH+ZKTk", "sceNpServiceChecker2IntGetServiceFlagArray", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("-Afi-JoRZ-U", "sceNpServiceChecker2IntInitialize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("ukBq62OPAYA", "sceNpServiceChecker2IntIsSetServiceType", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("waeEzwwYfZY", "sceNpServiceCheckerIntAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("YLXt-vGw4Kg", "sceNpServiceCheckerIntCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("85ZWdzWYgas", "sceNpServiceCheckerIntDestroyRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("LSQ3xApEoxY", "sceNpServiceCheckerIntFinalize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("wIX00Brskoc", "sceNpServiceCheckerIntGetAvailability", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("MjOFdwXYRKY", "sceNpServiceCheckerIntGetAvailabilityList", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("rT9Yk55JGho", "sceNpServiceCheckerIntInitialize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("az7fl9snOqw", "sceNpServiceCheckerIntIsCached", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("JHOtNtQ-jmw", "sceNpServiceClientInit", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("Hhmu86aYI1E", "sceNpServiceClientTerm", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("Y797Sw9-jqY", "sceNpAppInfoIntAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("UUhI+IUMrcE", "sceNpAppInfoIntCheckAvailability", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("ASonnwltwEk", "sceNpAppInfoIntCheckAvailabilityA", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("jXx0+2Wd1q8", "sceNpAppInfoIntCheckAvailabilityAll", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("f1OwQ7jdqn0", "sceNpAppInfoIntCheckAvailabilityAllA", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("1mfDBl40Dms", "sceNpAppInfoIntCheckServiceAvailability", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("XAmDowAQhFs", "sceNpAppInfoIntCheckServiceAvailabilityA", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("BaihFa8LBw0", "sceNpAppInfoIntCheckServiceAvailabilityAll", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("JcqdKidhuK0", "sceNpAppInfoIntCheckServiceAvailabilityAllA", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("cXpyESo49ko", "sceNpAppInfoIntCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("pRgpBtHx8P4", "sceNpAppInfoIntDestroyRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("s9+zoKE8cBA", "sceNpAppInfoIntFinalize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("l6Dl+2zlua0", "sceNpAppInfoIntInitialize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("od0pFsDoez0", "sceNpAppLauncherInitialize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("GbayQ7DO8jA", "sceNpAppLauncherLaunchApp", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("sy3PioM8TPE", "sceNpAppLauncherTerminate", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("OHCO6MMFvdQ", "sceNpAppLaunchLink2IntAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("B6IXdHGBL-g", "sceNpAppLaunchLink2IntCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("0H0JBpVp03o", "sceNpAppLaunchLink2IntDestroyRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("FWonlDV6d5k", "sceNpAppLaunchLink2IntFinalize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("PdYx470F6B8", "sceNpAppLaunchLink2IntGetCompatibleTitleIdList", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("tesM6ViaX6M", "sceNpAppLaunchLink2IntGetCompatibleTitleIdNum", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("DK6xpBP1gxw", "sceNpAppLaunchLink2IntInitialize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("AQV4A8YFx44", "sceNpAppLaunchLinkIntAbortRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("9YhwG4DhwtU", "sceNpAppLaunchLinkIntCreateRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("-8Wn4YKZLMM", "sceNpAppLaunchLinkIntDestroyRequest", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("TnQqJsyek5o", "sceNpAppLaunchLinkIntFinalize", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("GB7Fhk5SUaA", "sceNpAppLaunchLinkIntGetCompatibleTitleIdList", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("X4elOoiAtB4", "sceNpAppLaunchLinkIntGetCompatibleTitleIdNum", "libSceNpUtility", "libSceNpUtility");
    module->addSymbolStub("1F4yweQoqgg", "sceNpAppLaunchLinkIntInitialize", "libSceNpUtility", "libSceNpUtility");

    // libSceNpCommon
    module->addSymbolStub("j7DlalBzHh8", "sceShareUtilityInitializeEx2", "libSceShareUtility", "libSceShareUtility");

    // libSceNpGameIntent
    module->addSymbolStub("rPl0INNc-M8", "sceNpGameIntentGetPropertyValueString", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("KNggmcdqc2Q", "sceNpGameIntentGetUdsNpComId", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("m87BHxt-H60", "sceNpGameIntentInitialize", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("DAR+Nuv4E7M", "sceNpGameIntentLaunchApp", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("d-YDTQrxDJA", "sceNpGameIntentLaunchApp2", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("opFs796vTHg", "sceNpGameIntentNotifyAppLaunched", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("jEIXUAr9XE8", "sceNpGameIntentReceiveIntent", "libSceNpGameIntent", "libSceNpGameIntent");
    module->addSymbolStub("0HBYxYAjmf0", "sceNpGameIntentTerminate", "libSceNpGameIntent", "libSceNpGameIntent");
    
    // libSceAudioIn
    module->addSymbolExport("LozEOU8+anM", "sceAudioInInput", "libSceAudioIn", "libSceAudioIn", (void*)&sceAudioInInput);
    module->addSymbolStub("5NE8Sjc7VC8", "sceAudioInOpen", "libSceAudioIn", "libSceAudioIn", 1);
    module->addSymbolStub("IQtWgnrw6v8", "sceAudioInChangeAppModuleState", "libSceAudioIn", "libSceAudioIn");
    
    // libSceVoice
    module->addSymbolExport("CrLqDwWLoXM", "sceVoiceGetPortInfo", "libSceVoice", "libSceVoice", (void*)&sceVoiceGetPortInfo);
    module->addSymbolStub("9TrhuGzberQ", "sceVoiceInit", "libSceVoice", "libSceVoice");
    module->addSymbolStub("nXpje5yNpaE", "sceVoiceCreatePort", "libSceVoice", "libSceVoice");
    module->addSymbolStub("54phPH2LZls", "sceVoiceStart", "libSceVoice", "libSceVoice");
    module->addSymbolStub("oV9GAdJ23Gw", "sceVoiceConnectIPortToOPort", "libSceVoice", "libSceVoice");
    module->addSymbolStub("elcxZTEfHZM", "sceVoiceGetPortAttr", "libSceVoice", "libSceVoice");
    module->addSymbolStub("QBFoAIjJoXQ", "sceVoiceSetVolume", "libSceVoice", "libSceVoice");
    module->addSymbolStub("cJLufzou6bc", "sceVoiceGetBitRate", "libSceVoice", "libSceVoice");
    module->addSymbolStub("cQ6DGsQEjV4", "sceVoiceReadFromOPort", "libSceVoice", "libSceVoice");

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
    module->addSymbolStub("OIIm19xu+NM", "sceGameLiveStreamingGetProgramInfo", "libSceGameLiveStreaming", "libSceGameLiveStreaming");
    
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
    module->addSymbolStub("mrNh78tBpmg", "sceRemoteplayProhibit", "libSceRemoteplay", "libSceRemoteplay");
    module->addSymbolStub("g3PNjYKWqnQ", "sceRemoteplayGetConnectionStatus", "libSceRemoteplay", "libSceRemoteplay");
    module->addSymbolStub("7QLrixwVHcU", "sceRemoteplayProhibitStreaming", "libSceRemoteplay", "libSceRemoteplay");
    
    // libSceIme
    module->addSymbolStub("uTW+63goeJs", "InitializeImeModule", "libSceIme", "libSceIme");
    module->addSymbolStub("eaFXjfJv3xs", "sceImeKeyboardOpen", "libSceIme", "libSceIme");
    module->addSymbolStub("-4GCfYdNF1s", "sceImeUpdate", "libSceIme", "libSceIme");
    module->addSymbolStub("VkqLPArfFdc", "sceImeKeyboardGetInfo", "libSceIme", "libSceIme");
    module->addSymbolStub("dKadqZFgKKQ", "sceImeKeyboardGetResourceId", "libSceIme", "libSceIme");
    
    // libSceAvPlayer
    module->addSymbolStub("aS66RI0gGgo", "sceAvPlayerInit", "libSceAvPlayer", "libSceAvPlayer");
    
    // libSceRudp
    module->addSymbolStub("amuBfI-AQc4", "sceRudpInit", "libSceRudp", "libSceRudp");
    module->addSymbolStub("6PBNpsgyaxw", "sceRudpEnableInternalIOThread", "libSceRudp", "libSceRudp");
    module->addSymbolStub("SUEVes8gvmw", "sceRudpSetEventHandler", "libSceRudp", "libSceRudp");
    
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
    
    // libSceAudiodec
    module->addSymbolStub("VjhsmxpcezI", "sceAudiodecInitLibrary", "libSceAudiodec", "libSceAudiodec");
    module->addSymbolStub("h5jSB2QIDV0", "sceAudiodecTermLibrary", "libSceAudiodec", "libSceAudiodec");
    
    // libSceVisionManager
    module->addSymbolExport("UezlBvGQZUI", "sceVisionManagerGetWorkingMemorySize", "libSceVisionManager", "libSceVisionManager", (void*)&sceVisionManagerGetWorkingMemorySize);
    module->addSymbolStub("DphIqi0q48w", "sceVisionManagerGetLibraryVersion", "libSceVisionManager", "libSceVisionManager");
    module->addSymbolStub("6vmrNgzv924", "sceVisionManagerGetRegisterUserDataSize", "libSceVisionManager", "libSceVisionManager");
    module->addSymbolStub("3OLHl6cAPjQ", "sceVisionManagerGetRegisterUserDataVersion", "libSceVisionManager", "libSceVisionManager");
    module->addSymbolStub("9I4TdP3A9-g", "sceVisionManagerInitialize", "libSceVisionManager", "libSceVisionManager");
    
    // libSceAsyncStorageInternal
    module->addSymbolStub("AdTjrblPbkA", "libSceAsyncStorageInternalAux_AdTjrblPbkA", "libSceAsyncStorageInternalAux", "libSceAsyncStorageInternal");
    
    // libSceIpmi
    module->addSymbolStub("fjPNqzuUop8", "libSceIpmi_fjPNqzuUop8", "libSceIpmi", "libSceIpmi");
    
    // libSceUpdateService
    module->addSymbolStub("IOC0zyNzTM0", "sceUpsrvInitialize", "libSceUpdateService", "libSceUpdateService");
    
    // libScePatchCheckerClient
    module->addSymbolStub("m8oc1t4Rp28", "scePatchCheckerInitialize", "libScePatchCheckerClient", "libScePatchCheckerClient");
    
    // libSceRnpsAppMgr
    module->addSymbolStub("J7bWqy7TiBY", "sceRnpsAppMgrStartService", "libSceRnpsAppMgr", "libSceRnpsAppMgr");
    
    // libSceBgft
    module->addSymbolStub("BZ0olR8Da0g", "sceBgftServiceIntInit", "libSceBgft", "libSceBgft");
    module->addSymbolStub("vJhYrkgTYWY", "sceBgftServiceIntGetNotificationEvent", "libSceBgft", "libSceBgft", 0x80020055 /* no event? */);
    
    // libSceCdlgUtilServer
    module->addSymbolStub("33zrWSbHxcI", "libSceCdlgUtilServer_33zrWSbHxcI", "libSceCdlgUtilServer", "libSceCdlgUtilServer");
    
    // libSceKbEmulate
    module->addSymbolExport("mFR2QN8HNVU", "sceKeyboardPadEmulateOpen", "libSceKbEmulate", "libSceKbEmulate", (void*)&sceKeyboardPadEmulateOpen);

    // libSceHmd
    module->addSymbolStub("gEokC+OGI8g", "sceHmdDistortionGet2dVrCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ER2ar8yUmbk", "sceHmdDistortionGetCompoundEyeCorrectionCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("HT8qWOTOGmo", "sceHmdDistortionGetCorrectionCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Vkkhy8RFIuk", "sceHmdDistortionGetWideNearCorrectionCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("1cS7W5J-v3k", "sceHmdDistortionGetWorkMemoryAlign", "libSceHmd", "libSceHmd");
    module->addSymbolStub("36xDKk+Hw7o", "sceHmdDistortionGetWorkMemorySize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ao8NZ+FRYJE", "sceHmdDistortionInitialize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("8A4T5ahi790", "sceHmdDistortionSetOutputMinColor", "libSceHmd", "libSceHmd");
    module->addSymbolStub("bjYjvRCluuw", "sceHmdFillDistortionBuffer", "libSceHmd", "libSceHmd");
    module->addSymbolStub("liKOlmonGMo", "sceHmdGet2dDistortionMap", "libSceHmd", "libSceHmd");
    module->addSymbolStub("BWY-qKM5hxE", "sceHmdGet2DEyeOffset", "libSceHmd", "libSceHmd");
    module->addSymbolStub("za4xJfzCBcM", "sceHmdGet2dVrCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Yx+CuF11D3Q", "sceHmdGetAssyError", "libSceHmd", "libSceHmd");
    module->addSymbolStub("thDt9upZlp8", "sceHmdGetDeviceInformation", "libSceHmd", "libSceHmd");
    module->addSymbolStub("1pxQfif1rkE", "sceHmdGetDeviceInformationByHandle", "libSceHmd", "libSceHmd");
    module->addSymbolStub("grCYks4m8Jw", "sceHmdGetDistortionCorrectionCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("jWKiCTjC-us", "sceHmdGetDistortionCorrectionCommand2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("WyPdBHkHO7o", "sceHmdGetDistortionCorrectionCommandApprox", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Uu4VU1bY2Eo", "sceHmdGetDistortionMap", "libSceHmd", "libSceHmd");
    module->addSymbolStub("mP2ZcYmDg-o", "sceHmdGetDistortionParams", "libSceHmd", "libSceHmd");
    module->addSymbolStub("8Ick-e6cDVY", "sceHmdGetDistortionWorkMemoryAlign", "libSceHmd", "libSceHmd");
    module->addSymbolStub("SLvuzd81niM", "sceHmdGetDistortionWorkMemoryAlignFor2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("D5JfdpJKvXk", "sceHmdGetDistortionWorkMemorySize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("LJGdHRE3ui0", "sceHmdGetDistortionWorkMemorySizeFor2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("PyDpx1eHD8Y", "sceHmdGetEyeStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NPQwYFqi0bs", "sceHmdGetFieldOfView", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Wj4zVVejNOE", "sceHmdGetFieldOfView2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("g42bgpVPZXw", "sceHmdGetFieldOfViewApprox", "libSceHmd", "libSceHmd");
    module->addSymbolStub("rU3HK9Q0r8o", "sceHmdGetInertialSensorData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("6vWY0aegNnk", "sceHmdGetViewStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("goi5ASvH-V8", "sceHmdGetWideNearDistortionCorrectionCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("K4KnH0QkT2c", "sceHmdInitialize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("s-J66ar9g50", "sceHmdInitialize315", "libSceHmd", "libSceHmd");
    module->addSymbolStub("riPQfAdebHk", "sceHmdInternal3dAudioClose", "libSceHmd", "libSceHmd");
    module->addSymbolStub("wHnZU1qtiqw", "sceHmdInternal3dAudioOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NuEjeN8WCBA", "sceHmdInternal3dAudioSendData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("QasPTUPWVZE", "sceHmdInternalAnotherScreenClose", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Wr5KVtyVDG0", "sceHmdInternalAnotherScreenGetAudioStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("whRxl6Hhrzg", "sceHmdInternalAnotherScreenGetFadeState", "libSceHmd", "libSceHmd");
    module->addSymbolStub("w8BEUsIYn8w", "sceHmdInternalAnotherScreenGetVideoStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("0cQDAbkOt2A", "sceHmdInternalAnotherScreenOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Asczi8gw1NM", "sceHmdInternalAnotherScreenSendAudio", "libSceHmd", "libSceHmd");
    module->addSymbolStub("6+v7m1vwE+0", "sceHmdInternalAnotherScreenSendVideo", "libSceHmd", "libSceHmd");
    module->addSymbolStub("E0BLvy57IiQ", "sceHmdInternalAnotherScreenSetFadeAndSwitch", "libSceHmd", "libSceHmd");
    module->addSymbolStub("UTqrWB+1+SU", "sceHmdInternalBindDeviceWithUserId", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ego1YdqNGpI", "sceHmdInternalCheckDeviceModelMk3", "libSceHmd", "libSceHmd");
    module->addSymbolStub("WR7XsLdjcqQ", "sceHmdInternalCheckS3dPassModeAvailable", "libSceHmd", "libSceHmd");
    module->addSymbolStub("eMI1Hq+NEwY", "sceHmdInternalCrashReportCancel", "libSceHmd", "libSceHmd");
    module->addSymbolStub("dI3StPLQlMM", "sceHmdInternalCrashReportClose", "libSceHmd", "libSceHmd");
    module->addSymbolStub("lqPT-Bf1s4I", "sceHmdInternalCrashReportOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("QxhJs6zHUmU", "sceHmdInternalCrashReportReadData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("A2jWOLPzHHE", "sceHmdInternalCrashReportReadDataSize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("E9scVxt0DNg", "sceHmdInternalCreateSharedMemory", "libSceHmd", "libSceHmd");
    module->addSymbolStub("6RclvsKxr3I", "sceHmdInternalDfuCheckAfterPvt", "libSceHmd", "libSceHmd");
    module->addSymbolStub("cE99PJR6b8w", "sceHmdInternalDfuCheckPartialUpdateAvailable", "libSceHmd", "libSceHmd");
    module->addSymbolStub("SuE90Qscg0s", "sceHmdInternalDfuClose", "libSceHmd", "libSceHmd");
    module->addSymbolStub("5f-6lp7L5cY", "sceHmdInternalDfuGetStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("dv2RqD7ZBd4", "sceHmdInternalDfuOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("pN0HjRU86Jo", "sceHmdInternalDfuReset", "libSceHmd", "libSceHmd");
    module->addSymbolStub("mdc++HCXSsQ", "sceHmdInternalDfuSend", "libSceHmd", "libSceHmd");
    module->addSymbolStub("gjyqnphjGZE", "sceHmdInternalDfuSendSize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("bl4MkWNLxKs", "sceHmdInternalDfuSetMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("a1LmvXhZ6TM", "sceHmdInternalDfuStart", "libSceHmd", "libSceHmd");
    module->addSymbolStub("+UzzSnc0z9A", "sceHmdInternalEventInitialize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("uQc9P8Hrr6U", "sceHmdInternalGetBrightness", "libSceHmd", "libSceHmd");
    module->addSymbolStub("nK1g+MwMV10", "sceHmdInternalGetCrashDumpInfo", "libSceHmd", "libSceHmd");
    module->addSymbolStub("L5WZgOTw41Y", "sceHmdInternalGetDebugMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("3w8SkMfCHY0", "sceHmdInternalGetDebugSocialScreenMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("1Xmb76MHXug", "sceHmdInternalGetDebugTextMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("S0ITgPRkfUg", "sceHmdInternalGetDefaultLedData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("mxjolbeBa78", "sceHmdInternalGetDemoMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("RFIi20Wp9j0", "sceHmdInternalGetDeviceInformation", "libSceHmd", "libSceHmd");
    module->addSymbolStub("P04LQJQZ43Y", "sceHmdInternalGetDeviceInformationByHandle", "libSceHmd", "libSceHmd");
    module->addSymbolStub("PPCqsD8B5uM", "sceHmdInternalGetDeviceStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("-u82z1UhOq4", "sceHmdInternalGetEyeStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("iINSFzCIaB8", "sceHmdInternalGetHmuOpticalParam", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Csuvq2MMXHU", "sceHmdInternalGetHmuPowerStatusForDebug", "libSceHmd", "libSceHmd");
    module->addSymbolStub("UhFPniZvm8U", "sceHmdInternalGetHmuSerialNumber", "libSceHmd", "libSceHmd");
    module->addSymbolStub("9exeDpk7JU8", "sceHmdInternalGetIPD", "libSceHmd", "libSceHmd");
    module->addSymbolStub("yNtYRsxZ6-A", "sceHmdInternalGetIpdSettingEnableForSystemService", "libSceHmd", "libSceHmd");
    module->addSymbolStub("EKn+IFVsz0M", "sceHmdInternalGetPuBuildNumber", "libSceHmd", "libSceHmd");
    module->addSymbolStub("AxQ6HtktYfQ", "sceHmdInternalGetPuPositionParam", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ynKv9QCSbto", "sceHmdInternalGetPuRevision", "libSceHmd", "libSceHmd");
    module->addSymbolStub("3jcyx7XOm7A", "sceHmdInternalGetPUSerialNumber", "libSceHmd", "libSceHmd");
    module->addSymbolStub("+PDyXnclP5w", "sceHmdInternalGetPUVersion", "libSceHmd", "libSceHmd");
    module->addSymbolStub("67q17ERGBuw", "sceHmdInternalGetRequiredPUPVersion", "libSceHmd", "libSceHmd");
    module->addSymbolStub("aQDrTFzGkg8", "sceHmdInternalGetSensorCalibrationData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("uGyN1CkvwYU", "sceHmdInternalGetStatusReport", "libSceHmd", "libSceHmd");
    module->addSymbolStub("p9lSvZujLuo", "sceHmdInternalGetTv4kCapability", "libSceHmd", "libSceHmd");
    module->addSymbolStub("-Z+-9u98m9o", "sceHmdInternalGetVirtualDisplayDepth", "libSceHmd", "libSceHmd");
    module->addSymbolStub("df+b0FQnnVQ", "sceHmdInternalGetVirtualDisplayHeight", "libSceHmd", "libSceHmd");
    module->addSymbolStub("i6yROd9ygJs", "sceHmdInternalGetVirtualDisplaySize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Aajiktl6JXU", "sceHmdInternalGetVr2dData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("GwFVF2KkIT4", "sceHmdInternalIsCommonDlgMiniAppVr2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("LWQpWHOSUvk", "sceHmdInternalIsCommonDlgVr2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("YiIVBPLxmfE", "sceHmdInternalIsGameVr2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("LMlWs+oKHTg", "sceHmdInternalIsMiniAppVr2d", "libSceHmd", "libSceHmd");
    module->addSymbolStub("nBv4CKUGX0Y", "sceHmdInternalMapSharedMemory", "libSceHmd", "libSceHmd");
    module->addSymbolStub("4hTD8I3CyAk", "sceHmdInternalMirroringModeSetAspect", "libSceHmd", "libSceHmd");
    module->addSymbolStub("EJwPtSSZykY", "sceHmdInternalMirroringModeSetAspectDebug", "libSceHmd", "libSceHmd");
    module->addSymbolStub("r7f7M5q3snU", "sceHmdInternalMmapGetCount", "libSceHmd", "libSceHmd");
    module->addSymbolStub("gCjTEtEsOOw", "sceHmdInternalMmapGetModeId", "libSceHmd", "libSceHmd");
    module->addSymbolStub("HAr740Mt9Hs", "sceHmdInternalMmapGetSensorCalibrationData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("1PNiQR-7L6k", "sceHmdInternalMmapIsConnect", "libSceHmd", "libSceHmd");
    module->addSymbolStub("9-jaAXUNG-A", "sceHmdInternalPushVr2dData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("1gkbLH5+kxU", "sceHmdInternalRegisterEventCallback", "libSceHmd", "libSceHmd");
    module->addSymbolStub("6kHBllapJas", "sceHmdInternalResetInertialSensor", "libSceHmd", "libSceHmd");
    module->addSymbolStub("k1W6RPkd0mc", "sceHmdInternalResetLedForVrTracker", "libSceHmd", "libSceHmd");
    module->addSymbolStub("dp1wu22jSGc", "sceHmdInternalResetLedForVsh", "libSceHmd", "libSceHmd");
    module->addSymbolStub("d2TeoKeqM5U", "sceHmdInternalSeparateModeClose", "libSceHmd", "libSceHmd");
    module->addSymbolStub("WxsnAsjPF7Q", "sceHmdInternalSeparateModeGetAudioStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("eOOeG9SpEuc", "sceHmdInternalSeparateModeGetVideoStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("gA4Xnn+NSGk", "sceHmdInternalSeparateModeOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("stQ7AsondmE", "sceHmdInternalSeparateModeSendAudio", "libSceHmd", "libSceHmd");
    module->addSymbolStub("jfnS-OoDayM", "sceHmdInternalSeparateModeSendVideo", "libSceHmd", "libSceHmd");
    module->addSymbolStub("roHN4ml+tB8", "sceHmdInternalSetBrightness", "libSceHmd", "libSceHmd");
    module->addSymbolStub("0z2qLqedQH0", "sceHmdInternalSetCrashReportCommand", "libSceHmd", "libSceHmd");
    module->addSymbolStub("xhx5rVZEpnw", "sceHmdInternalSetDebugGpo", "libSceHmd", "libSceHmd");
    module->addSymbolStub("e7laRxRGCHc", "sceHmdInternalSetDebugMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("CRyJ7Q-ap3g", "sceHmdInternalSetDebugSocialScreenMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("dG4XPW4juU4", "sceHmdInternalSetDebugTextMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("rAXmGoO-VmE", "sceHmdInternalSetDefaultLedData", "libSceHmd", "libSceHmd");
    module->addSymbolStub("lu9I7jnUvWQ", "sceHmdInternalSetDemoMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("hyATMTuQSoQ", "sceHmdInternalSetDeviceConnection", "libSceHmd", "libSceHmd");
    module->addSymbolStub("c4mSi64bXUw", "sceHmdInternalSetForcedCrash", "libSceHmd", "libSceHmd");
    module->addSymbolStub("U9kPT4g1mFE", "sceHmdInternalSetHmuPowerControl", "libSceHmd", "libSceHmd");
    module->addSymbolStub("dX-MVpXIPwQ", "sceHmdInternalSetHmuPowerControlForDebug", "libSceHmd", "libSceHmd");
    module->addSymbolStub("4KIjvAf8PCA", "sceHmdInternalSetIPD", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NbxTfUKO184", "sceHmdInternalSetIpdSettingEnableForSystemService", "libSceHmd", "libSceHmd");
    module->addSymbolStub("HzrIn-Voa5o", "sceHmdInternalSetIPDSettingEnableForSystemService", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NnRKjf+hxW4", "sceHmdInternalSetLedOn", "libSceHmd", "libSceHmd");
    module->addSymbolStub("4AP0X9qGhqw", "sceHmdInternalSetM2LedBrightness", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Mzzz2HPWM+8", "sceHmdInternalSetM2LedOn", "libSceHmd", "libSceHmd");
    module->addSymbolStub("LkBkse9Pit0", "sceHmdInternalSetPortConnection", "libSceHmd", "libSceHmd");
    module->addSymbolStub("v243mvYg0Y0", "sceHmdInternalSetPortStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("EwXvkZpo9Go", "sceHmdInternalSetS3dPassMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("g3DKNOy1tYw", "sceHmdInternalSetSidetone", "libSceHmd", "libSceHmd");
    module->addSymbolStub("mjMsl838XM8", "sceHmdInternalSetUserType", "libSceHmd", "libSceHmd");
    module->addSymbolStub("8IS0KLkDNQY", "sceHmdInternalSetVirtualDisplayDepth", "libSceHmd", "libSceHmd");
    module->addSymbolStub("afhK5KcJOJY", "sceHmdInternalSetVirtualDisplayHeight", "libSceHmd", "libSceHmd");
    module->addSymbolStub("+zPvzIiB+BU", "sceHmdInternalSetVirtualDisplaySize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("9z8Lc64NF1c", "sceHmdInternalSetVRMode", "libSceHmd", "libSceHmd");
    module->addSymbolStub("s5EqYh5kbwM", "sceHmdInternalSocialScreenGetFadeState", "libSceHmd", "libSceHmd");
    module->addSymbolStub("a1LMFZtK9b0", "sceHmdInternalSocialScreenSetFadeAndSwitch", "libSceHmd", "libSceHmd");
    module->addSymbolStub("-6FjKlMA+Yc", "sceHmdInternalSocialScreenSetOutput", "libSceHmd", "libSceHmd");
    module->addSymbolStub("d2g5Ij7EUzo", "sceHmdOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NTIbBpSH9ik", "sceHmdReprojectionAddDisplayBuffer", "libSceHmd", "libSceHmd");
    module->addSymbolStub("94+Ggm38KCg", "sceHmdReprojectionClearUserEventEnd", "libSceHmd", "libSceHmd");
    module->addSymbolStub("mdyFbaJj66M", "sceHmdReprojectionClearUserEventStart", "libSceHmd", "libSceHmd");
    module->addSymbolStub("MdV0akauNow", "sceHmdReprojectionDebugGetLastInfo", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ymiwVjPB5+k", "sceHmdReprojectionDebugGetLastInfoMultilayer", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ZrV5YIqD09I", "sceHmdReprojectionFinalize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("utHD2Ab-Ixo", "sceHmdReprojectionFinalizeCapture", "libSceHmd", "libSceHmd");
    module->addSymbolStub("OuygGEWkins", "sceHmdReprojectionInitialize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("BTrQnC6fcAk", "sceHmdReprojectionInitializeCapture", "libSceHmd", "libSceHmd");
    module->addSymbolStub("TkcANcGM0s8", "sceHmdReprojectionQueryGarlicBuffAlign", "libSceHmd", "libSceHmd");
    module->addSymbolStub("z0KtN1vqF2E", "sceHmdReprojectionQueryGarlicBuffSize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("IWybWbR-xvA", "sceHmdReprojectionQueryOnionBuffAlign", "libSceHmd", "libSceHmd");
    module->addSymbolStub("kLUAkN6a1e8", "sceHmdReprojectionQueryOnionBuffSize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("6CRWGc-evO4", "sceHmdReprojectionSetCallback", "libSceHmd", "libSceHmd");
    module->addSymbolStub("E+dPfjeQLHI", "sceHmdReprojectionSetDisplayBuffers", "libSceHmd", "libSceHmd");
    module->addSymbolStub("LjdLRysHU6Y", "sceHmdReprojectionSetOutputMinColor", "libSceHmd", "libSceHmd");
    module->addSymbolStub("knyIhlkpLgE", "sceHmdReprojectionSetUserEventEnd", "libSceHmd", "libSceHmd");
    module->addSymbolStub("7as0CjXW1B8", "sceHmdReprojectionSetUserEventStart", "libSceHmd", "libSceHmd");
    module->addSymbolStub("3iONR2EXyKA", "sceHmdReprojectionSetUserEventToFinish", "libSceHmd", "libSceHmd");
    module->addSymbolStub("ro1JFV7JR+E", "sceHmdReprojectionSetUserEventToStart", "libSceHmd", "libSceHmd");
    module->addSymbolStub("dntZTJ7meIU", "sceHmdReprojectionStart", "libSceHmd", "libSceHmd");
    module->addSymbolStub("q3e8+nEguyE", "sceHmdReprojectionStart2dVr", "libSceHmd", "libSceHmd");
    module->addSymbolStub("RrvyU1pjb9A", "sceHmdReprojectionStartCapture", "libSceHmd", "libSceHmd");
    module->addSymbolStub("wbYAWZcJiNo", "sceHmdReprojectionStartCompoundEye", "libSceHmd", "libSceHmd");
    module->addSymbolStub("XZ5QUzb4ae0", "sceHmdReprojectionStartLiveCapture", "libSceHmd", "libSceHmd");
    module->addSymbolStub("8gH1aLgty5I", "sceHmdReprojectionStartMultilayer", "libSceHmd", "libSceHmd");
    module->addSymbolStub("gqAG7JYeE7A", "sceHmdReprojectionStartMultilayer2", "libSceHmd", "libSceHmd");
    module->addSymbolStub("3JyuejcNhC0", "sceHmdReprojectionStartWideNear", "libSceHmd", "libSceHmd");
    module->addSymbolStub("mKa8scOc4-k", "sceHmdReprojectionStartWideNearWithOverlay", "libSceHmd", "libSceHmd");
    module->addSymbolStub("kcldQ7zLYQQ", "sceHmdReprojectionStartWithOverlay", "libSceHmd", "libSceHmd");
    module->addSymbolStub("vzMEkwBQciM", "sceHmdReprojectionStop", "libSceHmd", "libSceHmd");
    module->addSymbolStub("F7Sndm5teWw", "sceHmdReprojectionStopCapture", "libSceHmd", "libSceHmd");
    module->addSymbolStub("PAa6cUL5bR4", "sceHmdReprojectionStopLiveCapture", "libSceHmd", "libSceHmd");
    module->addSymbolStub("0wnZViigP9o", "sceHmdReprojectionUnsetCallback", "libSceHmd", "libSceHmd");
    module->addSymbolStub("iGNNpDDjcwo", "sceHmdReprojectionUnsetDisplayBuffers", "libSceHmd", "libSceHmd");
    module->addSymbolStub("nmHzU4Gh0xs", "sceHmdSetupDialogClose", "libSceHmd", "libSceHmd");
    module->addSymbolStub("6lVRHMV5LY0", "sceHmdSetupDialogGetResult", "libSceHmd", "libSceHmd");
    module->addSymbolStub("J9eBpW1udl4", "sceHmdSetupDialogGetStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NB1Y2kA2jCY", "sceHmdSetupDialogInitialize", "libSceHmd", "libSceHmd");
    module->addSymbolStub("NNgiV4T+akU", "sceHmdSetupDialogOpen", "libSceHmd", "libSceHmd");
    module->addSymbolStub("+z4OJmFreZc", "sceHmdSetupDialogTerminate", "libSceHmd", "libSceHmd");
    module->addSymbolStub("Ud7j3+RDIBg", "sceHmdSetupDialogUpdateStatus", "libSceHmd", "libSceHmd");
    module->addSymbolStub("sWZSZB-mnw4", "libSceHmd_sWZSZB-mnw4", "libSceHmd", "libSceHmd");

    // libSceAvSetting
    module->addSymbolStub("t+O8mxM6oSg", "sceAvSettingAddCallbacks", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("us4sbukgU+w", "sceAvSettingAddCallbacksForLnc", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("x14XFcPNLJU", "sceAvSettingAddCallbacksForSocialScreen", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("PoeH5eI+ozw", "sceAvSettingCallbackFuncsInit", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("pSh4a1XK8eA", "sceAvSettingCallbackFuncsInit_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("DeucnsfJpqo", "sceAvSettingCallbackFuncsInitForLnc", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("aP6KEe871Ow", "sceAvSettingCallbackFuncsInitForLnc_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Ow348nbrI1I", "sceAvSettingChangeOutputMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("w7ICzzcAJCU", "sceAvSettingChangeOutputMode2", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("ZfMHgVDYzzY", "sceAvSettingChangeOutputMode3", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("gE40jgJgPsk", "sceAvSettingChangeOutputModeForDiag", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("EejsJSul9nA", "sceAvSettingChangeProcessAttribute", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("aKw9uBmZjpw", "sceAvSettingCheckCallback", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("85v1KZeMTgU", "sceAvSettingCloseLoopbackBuffers", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("5yga+o4TVqk", "sceAvSettingControlHdcpEncryption", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("oE4a8uIaXxI", "sceAvSettingDebugAddCallbacks", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("1DXOQcuiH8o", "sceAvSettingDebugClearDiagCommand", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("KfRNHvUILRQ", "sceAvSettingDebugGetDetailedHdcpStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("ME0ImfgjmL0", "sceAvSettingDebugSetDiagState", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("2X0ruzkCtrA", "sceAvSettingDebugSetHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("++DO8Y1JaYU", "sceAvSettingDebugSetProcessAttribute", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("dm0L3LVgQ+M", "sceAvSettingDriverChangeConnectionStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("XEGAcOsiVfQ", "sceAvSettingDriverUpdateStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("cejlyvC7+N8", "sceAvSettingEnterAudioMuteForShutdown", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("qnI61-kCm1E", "sceAvSettingGetCurrentDeviceInfo_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("1-O9j5vS8QU", "sceAvSettingGetCurrentHdmiDeviceId", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("PagHtqiYnQg", "sceAvSettingGetCurrentOutputMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("ZmPBwBD2tIY", "sceAvSettingGetCurrentOutputMode_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("j-b-RFZ3gjw", "sceAvSettingGetCurrentOutputMode2_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("vfzFZDaFuKU", "sceAvSettingGetDetailedHdcpStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("vh3aQ+JUpSU", "sceAvSettingGetDeviceInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("AoBZzDiZwng", "sceAvSettingGetHdcpStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("lxo162czs6I", "sceAvSettingGetHdmiConnectDisconnectNum", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("5ICXeCXAnDQ", "sceAvSettingGetHdmiKsvList", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("-Mui67TZd4s", "sceAvSettingGetHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("fgHOZo7gPyA", "sceAvSettingGetHdmiRawEdid", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("rjICp0cpHJM", "sceAvSettingGetLoopbackBuffer", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("6i0wQ7VpTSg", "sceAvSettingGetMorpheusPuStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("b40XbKKKDhQ", "sceAvSettingGetNativeHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("kJ0Nhf2At8Y", "sceAvSettingGetRawHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("EIZdWTT7Zdw", "sceAvSettingInit", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("l3dw8imUbLM", "sceAvSettingIsSupportedAudioOutModeByHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("WQVNCJkcmXc", "sceAvSettingIsSupportedHdcpVersionByHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Ej9YiEmOWVE", "sceAvSettingIsSupportedVideoOutModeByHdmiMonitorInfo", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("twiNZBeuppA", "sceAvSettingIsSuspendedProcessOutputModeAvailable", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("GUna2W6TBG4", "sceAvSettingIsSuspendedProcessOutputModeAvailable2", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Pjwx-8rnLT8", "sceAvSettingIsVrSupportedByHdmiMonitorInfo2", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("hdISXjL-bwY", "sceAvSettingNotifyAudioOutMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("0WVJD1NnpIU", "sceAvSettingNotifyDeviceEvent", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("RtwM77LoIrc", "sceAvSettingNotifyProcessPostResume", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Ft5agD1+fA0", "sceAvSettingNotifyProcessPostSuspend", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("T6LrBljciS0", "sceAvSettingNotifyProcessResume", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Ko8fB-6YDp8", "sceAvSettingNotifyProcessSuspend", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("02ozaKssy98", "sceAvSettingNotifyResponseForSocialScreen", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Fyw2Fkasc+4", "sceAvSettingNotifyUmdEvent", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("2cJVU9KC3zY", "sceAvSettingNotifyVideoOutMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Pxx1uH51g-E", "sceAvSettingOpenLoopbackBuffers", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("dnM9JfgxHbk", "sceAvSettingRemoveCallbacks", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("I37rBumJ9X4", "sceAvSettingRemoveProcessOutputMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("OEsO-6FT+04", "sceAvSettingSet2dVrMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("o7btxEpLl-0", "sceAvSettingSetAudioOutModeAny", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("r1f9mCImJCk", "sceAvSettingSetAudioOutModeAny_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("0hVwNAR5hz8", "sceAvSettingSetAudioOutModeInvalid", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("l1PGm+V7-TU", "sceAvSettingSetAudioOutModeInvalid_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("fscoS+Gh3Sw", "sceAvSettingSetAvOutputMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("YdgAk0w9rGY", "sceAvSettingSetDispclk", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("A-BYlKx72vw", "sceAvSettingSetDisplayBlackLevel", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("QGL+VQEX8lo", "sceAvSettingSetHdcpMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("+1rqC1b3Jw0", "sceAvSettingSetHdcpStatus", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("RFaqh8vAtb8", "sceAvSettingSetHdmiGamutMetadata_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("6s-tDu1w580", "sceAvSettingSetProcessAttribute", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("geoQ77m8Trc", "sceAvSettingSetVideoOutModeAny", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("t-BDxreA7sU", "sceAvSettingSetVideoOutModeAny_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("FSjfP0-ST3I", "sceAvSettingSetVideoOutModeInvalid", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("8RTK3rcM5aU", "sceAvSettingSetVideoOutModeInvalid_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("a4sYJjuBVbg", "sceAvSettingSetVideoOutputColorEffect", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("x-U4mJWRcNI", "sceAvSettingSetVideoOutSource", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("o0+89h9H3xA", "sceAvSettingSetVrMode", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("SKq7NiL8fA8", "sceAvSettingSimulateProcessOutputModeArbitration_", "libSceAvSetting", "libSceAvSetting");
    module->addSymbolStub("Xb2ez5SqR38", "sceAvSettingTerm", "libSceAvSetting", "libSceAvSetting");

    module->addSymbolStub("gdVNTX4s0XE", "sceLoginMgrServerCheckSafetyNoticeShown", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("qOlxBR2lpG0", "sceLoginMgrServerCheckTutorialShown", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("m0xKuqISvWY", "sceLoginMgrServerClearMorpheusRequiredUserId", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("VLB2F2owln0", "sceLoginMgrServerConvertToUserId", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("AT6NySjQ2gY", "sceLoginMgrServerCreateUser", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("REIRIXzv-H0", "sceLoginMgrServerDestroyUser", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("wZ2eXClFUaE", "sceLoginMgrServerDialogGetAppId", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("6+tPBogghdI", "sceLoginMgrServerDialogGetOpenParam", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("JkygqqJPGdw", "sceLoginMgrServerDialogNotifyCloseFinished", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("ci5qfgD5F58", "sceLoginMgrServerDialogNotifyOpenFinished", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("YMxfNRx4DR4", "sceLoginMgrServerDialogSetResult", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("K+WAuNzvn3g", "sceLoginMgrServerGetLogoutInfo", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("XpuZT1pS47A", "sceLoginMgrServerGetMorpheusRequiredUserId", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("7qkaKBAshAk", "sceLoginMgrServerGetMoveAssignmentModeForIDU", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("z4VKVtL7JbU", "sceLoginMgrServerGetSharePlayAllowPadOperation", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("4J3Z9DzaMzs", "sceLoginMgrServerInitialize", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("Xo9MQFVHpAQ", "sceLoginMgrServerInitializeSharePlayAllowPadOperation", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("mwCjW5FlkG4", "sceLoginMgrServerIsRequestedCdlgClose", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("P1su7LBMSUw", "sceLoginMgrServerLoginServiceGetRequestParam", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("bE03GJF7Bfc", "sceLoginMgrServerLoginServiceNotifyRequestFinished", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("dxpQVn+xVYE", "sceLoginMgrServerNotifyEasySignInFinished", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("GF+GkOCVVLg", "sceLoginMgrServerNotifyEasySignInUserCode", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("F+xsDVy+gqw", "sceLoginMgrServerNotifyHmdConnectInfo", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("ozddwj2QrKI", "sceLoginMgrServerNotifySafetyNoticeShown", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("LffcWEebPwg", "sceLoginMgrServerNotifyShellUIState", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("gmoxsDewVKI", "sceLoginMgrServerNotifyTutorialShown", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("Jha8yMoMzrg", "sceLoginMgrServerSetLoginFlag", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("DnC0qZ9h+Ms", "sceLoginMgrServerSetMorpheusAssignEnabledFlag", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("wkIxHrwZ3+M", "sceLoginMgrServerSetMorpheusRequiredUserId", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("KxitkrVcrSg", "sceLoginMgrServerSetSharePlayMode", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("-+h1C78SdyU", "sceLoginMgrServerSetUserStatus", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("NtHCBzSqxgo", "sceLoginMgrServerTerminate", "libSceLoginMgrServer", "libSceLoginMgrServer");
    module->addSymbolStub("NQY2wMTV0ms", "sceLoginMgrServerUpdateUserIdRalatedToPadUniqueId", "libSceLoginMgrServer", "libSceLoginMgrServer");
    
    // libSceContentExport
    module->addSymbolStub("efqHe7wPRbs", "sceContentExportCancel", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("tb3cZTCl8Ps", "sceContentExportFinish", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("AOWqIYsgVHs", "sceContentExportFromData", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("uZTQHI50WpY", "sceContentExportFromDataWithThumbnail", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("OWCJUmrWH1g", "sceContentExportFromFile", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("bU89EJ+j9f0", "sceContentExportFromFileWithContentIdList", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("mbakKLPSO4o", "sceContentExportFromFileWithThumbnail", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("HK-7ir0qAkg", "sceContentExportFromFileWithTitleIdList", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("eRJv4xU7pGU", "sceContentExportGetProgress", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("FzEWeYnAFlI", "sceContentExportInit", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("0GnN4QCgIfs", "sceContentExportInit2", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("FCygF4Ec4so", "sceContentExportStart", "libSceContentExport", "libSceContentExport");
    module->addSymbolStub("+KDWny9Y-6k", "sceContentExportTerm", "libSceContentExport", "libSceContentExport");
    
    // libSceContentSearch
    module->addSymbolStub("-YbpaF0XS-I", "sceContentSearchCloseMetadata", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("jGHBPci4dzU", "sceContentSearchGetApplicationLastUpdateId", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("PD5bnBTsv6Y", "sceContentSearchGetContentLastUpdateId", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("EbNufIY0Zvc", "sceContentSearchGetMetadataFieldInfo", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("ruNe-FgCzO8", "sceContentSearchGetMetadataValue", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("FRT4EYtZU1Y", "sceContentSearchGetMyApplicationIndex", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("BU0x8ye5-dI", "sceContentSearchGetNumOfApplication", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("o-RBPV0qr8c", "sceContentSearchGetNumOfContent", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("i2ZMZKwZpWs", "sceContentSearchGetTotalContentSize", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("dPj4ZtRcIWk", "sceContentSearchInit", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("G13HF1z-e4o", "sceContentSearchOpenMetadata", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("bjAlYWwRTJA", "sceContentSearchOpenMetadataByContentId", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("m-S3-C-USvs", "sceContentSearchSearchApplication", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("TEW3IKxYfXc", "sceContentSearchSearchContent", "libSceContentSearch", "libSceContentSearch");
    module->addSymbolStub("1xSZodB2geA", "sceContentSearchTerm", "libSceContentSearch", "libSceContentSearch");

    
    // libSceCompanionHttpd
    module->addSymbolStub("8pWltDG7h6A", "sceCompanionHttpdAddHeader", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("B-QBMeFdNgY", "sceCompanionHttpdGet2ndScreenStatus", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("Vku4big+IYM", "sceCompanionHttpdGetEvent", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("0SySxcuVNG0", "sceCompanionHttpdGetUserId", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("ykNpWs3ktLY", "sceCompanionHttpdInitialize", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("OA6FbORefbo", "sceCompanionHttpdInitialize2", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("r-2-a0c7Kfc", "sceCompanionHttpdOptParamInitialize", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("fHNmij7kAUM", "sceCompanionHttpdRegisterRequestBodyReceptionCallback", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("OaWw+IVEdbI", "sceCompanionHttpdRegisterRequestCallback", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("-0c9TCTwnGs", "sceCompanionHttpdRegisterRequestCallback2", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("h3OvVxzX4qM", "sceCompanionHttpdSetBody", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("w7oz0AWHpT4", "sceCompanionHttpdSetStatus", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("k7F0FcDM-Xc", "sceCompanionHttpdStart", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("0SCgzfVQHpo", "sceCompanionHttpdStop", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("+-du9tWgE9s", "sceCompanionHttpdTerminate", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("ZSHiUfYK+QI", "sceCompanionHttpdUnregisterRequestBodyReceptionCallback", "libSceCompanionHttpd", "libSceCompanionHttpd");
    module->addSymbolStub("xweOi2QT-BE", "sceCompanionHttpdUnregisterRequestCallback", "libSceCompanionHttpd", "libSceCompanionHttpd");


    return module;
}

}   // End namespace PS4::OS::HLE