#include "HLE.hpp"


namespace PS4::OS::HLE {

    // Create a dummy HLE module that only contains the symbol exports for HLE functions
    Module buildHLEModule() {
        Module module;

        PS4::OS::Libs::Kernel::init(module);
    
        return module;
    }

}   // End namespace PS4::OS::HLE