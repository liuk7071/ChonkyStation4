#include "mutex.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_mutex);

s32 PS4_FUNC kernel_pthread_mutex_lock(pthread_mutex_t* mutex) {
    log("pthread_mutex_lock(mutex=%p)\n", mutex);

    if (*mutex == 0) *mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;

    s32 ret = pthread_mutex_lock(mutex);
    return ret;
}

s32 PS4_FUNC kernel_pthread_mutex_trylock(pthread_mutex_t* mutex) {
    log("pthread_mutex_trylock(mutex=%p)\n", mutex);

    if (*mutex == 0) *mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;

    s32 ret = pthread_mutex_trylock(mutex);
    return ret;
}

s32 PS4_FUNC kernel_pthread_mutex_unlock(pthread_mutex_t* mutex) {
    log("pthread_mutex_unlock(mutex=%p)\n", mutex);
    return pthread_mutex_unlock(mutex);
}

s32 PS4_FUNC kernel_pthread_mutexattr_init(pthread_mutexattr_t* attr) {
    log("pthread_mutexattr_init(attr=%p)\n", attr);
    return pthread_mutexattr_init(attr);
}

s32 PS4_FUNC kernel_pthread_mutexattr_destroy(pthread_mutexattr_t* attr) {
    log("pthread_mutexattr_destroy(attr=%p)\n", attr);
    return pthread_mutexattr_destroy(attr);
}

s32 PS4_FUNC kernel_pthread_mutexattr_settype(pthread_mutexattr_t* attr, int kind) {
    log("pthread_mutexattr_settype(attr=%p, kind=%d)\n", attr, kind);
    return pthread_mutexattr_settype(attr, kind);
}

s32 PS4_FUNC kernel_pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {
    log("pthread_mutex_init(mutex=%p, attr=%p)\n", mutex, attr);

    if (!attr) {
        pthread_mutexattr_t new_attr;
        pthread_mutexattr_init(&new_attr);
        pthread_mutexattr_settype(&new_attr, PTHREAD_MUTEX_ERRORCHECK);
        return pthread_mutex_init(mutex, &new_attr);
    }

    return pthread_mutex_init(mutex, attr);
}

s32 PS4_FUNC kernel_pthread_mutex_destroy(pthread_mutex_t* mutex) {
    log("pthread_mutex_destroy(mutex=%p)\n", mutex);
    return pthread_mutex_destroy(mutex);
}

};  // End namespace PS4::OS::Libs::Kernel