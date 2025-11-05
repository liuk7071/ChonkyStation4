#include "mutex.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

s32 PS4_FUNC kernel_pthread_mutex_lock(pthread_mutex_t* mutex) {
    log("pthread_mutex_lock(mutex=%p)\n", mutex);

    if (*mutex == 0) *mutex = PTHREAD_MUTEX_INITIALIZER;

    s32 ret = pthread_mutex_lock(mutex);
    return ret;
}

s32 PS4_FUNC kernel_pthread_mutex_unlock(pthread_mutex_t* mutex) {
    log("pthread_mutex_unlock(mutex=%p)\n", mutex);
    return pthread_mutex_unlock(mutex);
}

};  // End namespace PS4::OS::Libs::Kernel