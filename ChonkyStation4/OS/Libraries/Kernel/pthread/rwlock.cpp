#include "rwlock.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

s32 PS4_FUNC scePthreadRwlockInit(pthread_rwlock_t* lock, const pthread_rwlockattr_t* attr, const char* name) {
    log("scePthreadRwlockInit(lock=*%p, attr=*%p, name=\"%s\")\n", lock, attr, name);
    return pthread_rwlock_init(lock, attr);
}

s32 PS4_FUNC kernel_pthread_rwlock_rdlock(pthread_rwlock_t* lock) {
    log("pthread_rwlock_rdlock(lock=*%p)\n", lock);
    return pthread_rwlock_rdlock(lock);
}

s32 PS4_FUNC kernel_pthread_rwlock_wrlock(pthread_rwlock_t* lock) {
    log("pthread_rwlock_wrlock(lock=*%p)\n", lock);
    return pthread_rwlock_wrlock(lock);
}

s32 PS4_FUNC kernel_pthread_rwlock_unlock(pthread_rwlock_t* lock) {
    log("pthread_rwlock_unlock(lock=*%p)\n", lock);
    return pthread_rwlock_unlock(lock);
}

s32 PS4_FUNC kernel_pthread_rwlockattr_init(pthread_rwlockattr_t* attr) {
    log("pthread_rwlockattr_init(attr=*%p)\n", attr);
    return pthread_rwlockattr_init(attr);
}

}   // End namespace PS4::OS::Libs::Kernel