#include "cond.hpp"
#include <Logger.hpp>
#include <Common/ErrorCodes.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

#ifdef _WIN32
#define CHECK_INIT if (*(int*)cond == 0) *cond = PTHREAD_COND_INITIALIZER;
#else
#define CHECK_INIT                              \
if (*(int*)cond == 0) {                         \
    *cond = new CondWrapper();                  \
    (*cond)->cond = PTHREAD_COND_INITIALIZER;   \
}
#endif

s32 PS4_FUNC kernel_pthread_condattr_init(CONDATTR_IMPL* attr) {
    log("pthread_condattr_init(attr=*%p)\n", attr);
#ifdef _WIN32
    s32 ret = pthread_condattr_init(attr);
#else
    *attr = new CondAttrWrapper();
    s32 ret = pthread_condattr_init(&(*attr)->attr);
#endif
    
    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_cond_init(COND_IMPL* cond, const CONDATTR_IMPL* attr) {
    log("pthread_cond_init(cond=*%p, attr=*%p)\n", cond, attr);
#ifdef _WIN32
    s32 ret = pthread_cond_init(cond, attr);
#else
    *cond = new CondWrapper();
    s32 ret = pthread_cond_init(&(*cond)->cond, attr ? &(*attr)->attr : nullptr);
#endif
    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC scePthreadCondInit(COND_IMPL* cond, const CONDATTR_IMPL* attr, const char* name) {
    log("scePthreadCondInit(cond=*%p, attr=*%p, name=\"%s\")\n", cond, attr, name);
    #ifdef _WIN32
    s32 ret = pthread_cond_init(cond, attr);
#else
    *cond = new CondWrapper();
    s32 ret = pthread_cond_init(&(*cond)->cond, attr ? &(*attr)->attr : nullptr);
#endif
    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_cond_wait(COND_IMPL* cond, MUTEX_IMPL* mutex) {
    log("pthread_cond_wait(cond=*%p, mutex=*%p)\n", cond, mutex);

    CHECK_INIT

#ifdef _WIN32
    s32 ret = pthread_cond_wait(cond, mutex);
#else
    s32 ret = pthread_cond_wait(&(*cond)->cond, &(*mutex)->mutex);
#endif
    
    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_cond_timedwait(COND_IMPL* cond, MUTEX_IMPL* mutex, const SceKernelTimespec* abstime) {
    log("pthread_cond_timedwait(cond=*%p, mutex=*%p, abstime=*%p)\n", cond, mutex, abstime);
    timespec time;
    //time.tv_sec  = abstime->tv_sec;
    //time.tv_nsec = abstime->tv_nsec;
    
    CHECK_INIT

    auto now = std::chrono::system_clock::now();
    auto timeout = now + std::chrono::milliseconds(100);
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(timeout);
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - secs);
    time.tv_sec = secs.time_since_epoch().count();
    time.tv_nsec = nsec.count();
    
#ifdef _WIN32
    return pthread_cond_timedwait(cond, mutex, &time);
#else 
    return pthread_cond_timedwait(&(*cond)->cond, &(*mutex)->mutex, &time);
#endif
}

s32 PS4_FUNC scePthreadCondTimedwait(COND_IMPL* cond, MUTEX_IMPL* mutex, u64 us) {
    log("scePthreadCondTimedwait(cond=*%p, mutex=*%p, us=%lld)\n", cond, mutex, us);

    CHECK_INIT

    auto now = std::chrono::system_clock::now();
    auto timeout = now + std::chrono::microseconds(us);
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(timeout);
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - secs);

    timespec time;
    time.tv_sec = secs.time_since_epoch().count();
    time.tv_nsec = nsec.count();

#ifdef _WIN32
    const auto ret = pthread_cond_timedwait(cond, mutex, &time);
#else
    const auto ret = pthread_cond_timedwait(&(*cond)->cond, &(*mutex)->mutex, &time);
#endif

    if (ret == ETIMEDOUT)
        return SCE_KERNEL_ERROR_ETIMEDOUT;
    else if (ret == 0)
        return 0;

    Helpers::panic("scePthreadCondTimedwait: unexpected error %d\n", ret);
}

s32 PS4_FUNC kernel_pthread_cond_signal(COND_IMPL* cond) {
    log("pthread_cond_signal(cond=*%p)\n", cond);

    CHECK_INIT

#ifdef _WIN32
    s32 ret = pthread_cond_signal(cond);
#else
    s32 ret = pthread_cond_signal(&(*cond)->cond);
#endif
    
    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_cond_broadcast(COND_IMPL* cond) {
    log("pthread_cond_broadcast(cond=*%p)\n", cond);

    CHECK_INIT
    
#ifdef _WIN32
    s32 ret = pthread_cond_broadcast(cond);
#else
    s32 ret = pthread_cond_broadcast(&(*cond)->cond);
#endif

    PTHREAD_CHECK_RESULT(ret);
    return ret;
}

s32 PS4_FUNC kernel_pthread_condattr_destroy(CONDATTR_IMPL* attr) {
    log("pthread_condattr_destroy(attr=*%p)\n", attr);
#ifdef _WIN32
    return pthread_condattr_destroy(attr);
#else
    s32 ret = pthread_condattr_destroy(&(*attr)->attr);
    delete *attr;
    return ret;
#endif
}

};  // End namespace PS4::OS::Libs::Kernel