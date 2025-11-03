#include "mutex.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

u32 PS4_FUNC kernel_pthread_mutex_lock(pthread_mutex_t* mutex) {
    log("pthread_mutex_lock(mutex=%p)\n", mutex);
    return pthread_mutex_lock(mutex);
}

u32 PS4_FUNC kernel_pthread_mutex_unlock(pthread_mutex_t* mutex) {
    log("pthread_mutex_unlock(mutex=%p)\n", mutex);
    return pthread_mutex_unlock(mutex);
}

};  // End namespace PS4::OS::Libs::Kernel