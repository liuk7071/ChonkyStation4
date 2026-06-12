#include "mutex.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_mutex);

#ifdef _WIN32
#define CHECK_INIT \
if (*mutex == (pthread_mutex_t)0 || *mutex == (pthread_mutex_t)1) {     \
    log("mutex was null, initializing\n");                              \
    *mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;                      \
    kernel_pthread_mutex_init(mutex, nullptr);                          \
}
#else
#define CHECK_INIT \
if (*(int*)mutex == 0 || *(int*)mutex == 1) {                           \
    log("mutex was null, initializing\n");                              \
    kernel_pthread_mutex_init(mutex, nullptr);                          \
}
#endif

s32 PS4_FUNC kernel_pthread_mutex_lock(MUTEX_IMPL* mutex) {
    log("pthread_mutex_lock(mutex=%p)\n", mutex);

    CHECK_INIT

#ifdef _WIN32
    auto ret = pthread_mutex_lock(mutex);
#else
    auto ret = pthread_mutex_lock(&(*mutex)->mutex);
#endif
    
    //PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_mutex_trylock(MUTEX_IMPL* mutex) {
    log("pthread_mutex_trylock(mutex=%p)\n", mutex);

    CHECK_INIT

#ifdef _WIN32
    auto ret = pthread_mutex_trylock(mutex);
#else
    auto ret = pthread_mutex_trylock(&(*mutex)->mutex);
#endif
    
    //PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC scePthreadMutexTimedlock(MUTEX_IMPL* mutex, u64 us) {
    log("scePthreadMutexTimedlock(mutex=%p, us=%lld)\n", mutex, us);

    CHECK_INIT

    timespec time;
    time.tv_sec  = us / 1000000;
    time.tv_nsec = (us % 1000000) * 1000;
    
#ifdef _WIN32
    auto ret = pthread_mutex_timedlock(mutex, &time);
#else
    auto ret = pthread_mutex_timedlock(&(*mutex)->mutex, &time);
#endif

    if (ret == ETIMEDOUT)
        return SCE_KERNEL_ERROR_ETIMEDOUT;

    //PTHREAD_CHECK_RESULT(ret);
    return SCE_OK;
}

s32 PS4_FUNC kernel_pthread_mutex_unlock(MUTEX_IMPL* mutex) {
    log("pthread_mutex_unlock(mutex=%p)\n", mutex);
#ifdef _WIN32   
    return pthread_mutex_unlock(mutex);
#else
    return pthread_mutex_unlock(&(*mutex)->mutex);
#endif
}

s32 PS4_FUNC kernel_pthread_mutexattr_init(MUTEXATTR_IMPL* attr) {
    log("pthread_mutexattr_init(attr=%p)\n", attr);
#ifdef _WIN32
    s32 ret = pthread_mutexattr_init(attr);
    pthread_mutexattr_settype(attr, PTHREAD_MUTEX_ERRORCHECK);
#else
    *attr = new MutexAttrWrapper();
    s32 ret = pthread_mutexattr_init(&(*attr)->attr);
    pthread_mutexattr_settype(&(*attr)->attr, PTHREAD_MUTEX_ERRORCHECK);
#endif    
    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_mutexattr_destroy(MUTEXATTR_IMPL* attr) {
    log("pthread_mutexattr_destroy(attr=%p)\n", attr);
#ifdef _WIN32
    return pthread_mutexattr_destroy(attr);
#else
    s32 ret = pthread_mutexattr_destroy(&(*attr)->attr);
    delete *attr;
    return ret;
#endif
}

s32 PS4_FUNC kernel_pthread_mutexattr_settype(MUTEXATTR_IMPL* attr, int kind) {
    log("pthread_mutexattr_settype(attr=%p, kind=%d)\n", attr, kind);
    
    switch (kind) {
    case 1: kind = PTHREAD_MUTEX_ERRORCHECK;    break;
    case 2: kind = PTHREAD_MUTEX_RECURSIVE;     break;
    case 3: kind = PTHREAD_MUTEX_NORMAL;        break;
    //case 4: kind = PTHREAD_MUTEX_ADAPTIVE_NP;   break;
    case 4: kind = PTHREAD_MUTEX_ERRORCHECK;    break;
    default:    Helpers::panic("pthread_mutexattr_settype: invalid type");
    }
    
#ifdef _WIN32
    return pthread_mutexattr_settype(attr, kind);
#else
    return pthread_mutexattr_settype(&(*attr)->attr, kind);
#endif
}

s32 PS4_FUNC kernel_pthread_mutex_init(MUTEX_IMPL* mutex, const MUTEXATTR_IMPL* attr) {
    log("pthread_mutex_init(mutex=%p, attr=%p)\n", mutex, attr);

#ifdef _WIN32
    pthread_mutexattr_t* in_attr = (pthread_mutexattr_t*)attr;
#else
    pthread_mutexattr_t* in_attr = attr ? (pthread_mutexattr_t*)&(*attr)->attr : nullptr;
#endif

    bool destroy_new_attr = false;
    pthread_mutexattr_t new_attr;
    if (!in_attr) {
        pthread_mutexattr_init(&new_attr);
        pthread_mutexattr_settype(&new_attr, PTHREAD_MUTEX_ERRORCHECK);
        in_attr = &new_attr;
        destroy_new_attr = true;
    }

#ifdef _WIN32
    s32 ret = pthread_mutex_init(mutex, in_attr);
#else
    *mutex = new MutexWrapper();
    s32 ret = pthread_mutex_init(&(*mutex)->mutex, in_attr);
#endif

    if (destroy_new_attr) {
        pthread_mutexattr_destroy(in_attr);
    }

    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_mutex_destroy(MUTEX_IMPL* mutex) {
    log("pthread_mutex_destroy(mutex=%p)\n", mutex);
#ifdef _WIN32
    return pthread_mutex_destroy(mutex);
#else
    s32 ret = pthread_mutex_destroy(&(*mutex)->mutex);
    delete *mutex;
    return ret;
#endif
}

};  // End namespace PS4::OS::Libs::Kernel