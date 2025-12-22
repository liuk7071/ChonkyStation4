#include "HLE.hpp"
#include <OS/Libraries/Kernel/kernel.hpp>
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


// Stub until we implement audio input
s32 sceAudioInInput(s32 handle, void* ptr) {
    while (true) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
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

    // libSceScreenShot
    module.addSymbolStub("73WQ4Jj0nJI", "sceScreenShotSetOverlayImageWithOrigin", "libSceScreenShot", "libSceScreenShot");
    
    // libSceMsgDialog
    module.addSymbolStub("lDqxaY1UbEo", "sceMsgDialogInitialize", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("b06Hh0DPEaE", "sceMsgDialogOpen", "libSceMsgDialog", "libSceMsgDialog");
    module.addSymbolStub("6fIC3XKt2k0", "sceMsgDialogUpdateStatus", "libSceMsgDialog", "libSceMsgDialog");

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
    
    // libSceErrorDialog
    module.addSymbolStub("I88KChlynSs", "sceErrorDialogInitialize", "libSceErrorDialog", "libSceErrorDialog", 0);
    module.addSymbolStub("M2ZF-ClLhgY", "sceErrorDialogOpen", "libSceErrorDialog", "libSceErrorDialog", 0);
    module.addSymbolStub("WWiGuh9XfgQ", "sceErrorDialogUpdateStatus", "libSceErrorDialog", "libSceErrorDialog", 0);
    
    // libSceNpProfileDialog
    module.addSymbolStub("Lg+NCE6pTwQ", "sceNpProfileDialogInitialize", "libSceNpProfileDialog", "libSceNpProfileDialog", 0);
    
    // libSceNet
    module.addSymbolStub("Nlev7Lg8k3A", "sceNetInit", "libSceNet", "libSceNet", 0);
    module.addSymbolStub("dgJBaeJnGpo", "sceNetPoolCreate", "libSceNet", "libSceNet", 1);

    // libSceSsl
    module.addSymbolStub("hdpVEUDFW3s", "sceSslInit", "libSceSsl", "libSceSsl", 1);
    
    // libSceHttp
    module.addSymbolStub("A9cVMUtEp4Y", "sceHttpInit", "libSceHttp", "libSceHttp", 1);
    
    // libSceAudioIn
    module.addSymbolStub("5NE8Sjc7VC8", "sceAudioInOpen", "libSceAudioIn", "libSceAudioIn", 1);
    module.addSymbolExport("LozEOU8+anM", "sceAudioInInput", "libSceAudioIn", "libSceAudioIn", (void*)&sceAudioInInput);
    
    return module;
}

}   // End namespace PS4::OS::HLE