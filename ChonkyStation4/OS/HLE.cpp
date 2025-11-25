#include "HLE.hpp"
#include <OS/Libraries/Kernel/kernel.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>
#include <OS/Libraries/SceGnmDriver/SceGnmDriver.hpp>
#include <OS/Libraries/SceSystemService/SceSystemService.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>
#include <OS/Libraries/SceNpManager/SceNpManager.hpp>
#include <OS/Libraries/SceSysmodule/SceSysmodule.hpp>


namespace PS4::OS::HLE {

    // Create a dummy HLE module that only contains the symbol exports for HLE functions
    Module buildHLEModule() {
        Module module;

        PS4::OS::Libs::Kernel::init(module);
        PS4::OS::Libs::SceVideoOut::init(module);
        PS4::OS::Libs::SceGnmDriver::init(module);
        PS4::OS::Libs::SceSystemService::init(module);
        PS4::OS::Libs::SceUserService::init(module);
        PS4::OS::Libs::SceNpManager::init(module);
        PS4::OS::Libs::SceSysmodule::init(module);

        // libSceScreenShot
        module.addSymbolStub("73WQ4Jj0nJI", "sceScreenShotSetOverlayImageWithOrigin", "libSceScreenShot", "libSceScreenShot");

        // libSceCommonDialog
        module.addSymbolStub("uoUpLGNkygk", "sceCommonDialogInitialize", "libSceCommonDialog", "libSceCommonDialog");
    
        return module;
    }

}   // End namespace PS4::OS::HLE