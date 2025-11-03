#include "cond.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

u32 PS4_FUNC kernel_pthread_cond_broadcast() {
    log("pthread_cond_broadcast() TODO\n");
    return 0;
}

};  // End namespace PS4::OS::Libs::Kernel