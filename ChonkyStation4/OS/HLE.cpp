#include "HLE.hpp"
#include <OS/Libraries/Kernel/kernel.hpp>
#include <OS/Libraries/SceVideoOut/SceVideoOut.hpp>


namespace PS4::OS::HLE {

    // Create a dummy HLE module that only contains the symbol exports for HLE functions
    Module buildHLEModule() {
        Module module;

        PS4::OS::Libs::Kernel::init(module);
        PS4::OS::Libs::SceVideoOut::init(module);
    
        return module;
    }

}   // End namespace PS4::OS::HLE