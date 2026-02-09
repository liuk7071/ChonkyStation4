#include "cond.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

s32 PS4_FUNC kernel_pthread_condattr_init(pthread_condattr_t* attr) {
    log("pthread_condattr_init(attr=*%p)\n", attr);
    return pthread_condattr_init(attr);
}

s32 PS4_FUNC kernel_pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr) {
    log("pthread_cond_init(cond=*%p, attr=*%p)\n", cond, attr);
    return pthread_cond_init(cond, attr);
}

s32 PS4_FUNC scePthreadCondInit(pthread_cond_t* cond, const pthread_condattr_t* attr, const char* name) {
    log("scePthreadCondInit(cond=*%p, attr=*%p, name=\"%s\")\n", cond, attr, name);
    return pthread_cond_init(cond, attr);
}

s32 PS4_FUNC kernel_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    log("pthread_cond_wait(cond=*%p, mutex=*%p)\n", cond, mutex);

    if (*cond == 0) *cond = PTHREAD_COND_INITIALIZER;
    return pthread_cond_wait(cond, mutex);
}

s32 PS4_FUNC kernel_pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const SceKernelTimespec* abstime) {
    log("pthread_cond_timedwait(cond=*%p, mutex=*%p, abstime=*%p)\n", cond, mutex, abstime);
    timespec time;
    //time.tv_sec  = abstime->tv_sec;
    //time.tv_nsec = abstime->tv_nsec;
    
    auto now = std::chrono::system_clock::now();
    auto timeout = now + std::chrono::milliseconds(100);
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(timeout);
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - secs);
    time.tv_sec = secs.time_since_epoch().count();
    time.tv_nsec = nsec.count();

    return pthread_cond_timedwait(cond, mutex, &time);
}

s32 PS4_FUNC scePthreadCondTimedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, u64 us) {
    log("scePthreadCondTimedwait(cond=*%p, mutex=*%p, us=%ulld)\n", cond, mutex, us);

    auto now = std::chrono::system_clock::now();
    auto timeout = now + std::chrono::microseconds(us);
    auto secs = std::chrono::time_point_cast<std::chrono::seconds>(timeout);
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - secs);

    timespec time;
    time.tv_sec = secs.time_since_epoch().count();
    time.tv_nsec = nsec.count();
    return pthread_cond_timedwait(cond, mutex, &time);
}

s32 PS4_FUNC kernel_pthread_cond_signal(pthread_cond_t* cond) {
    log("pthread_cond_signal(cond=*%p)\n", cond);

    if (*cond == 0) *cond = PTHREAD_COND_INITIALIZER;
    return pthread_cond_signal(cond);
}

s32 PS4_FUNC kernel_pthread_cond_broadcast(pthread_cond_t* cond) {
    log("pthread_cond_broadcast(cond=*%p)\n", cond);

    if (*cond == 0) *cond = PTHREAD_COND_INITIALIZER;
    return pthread_cond_broadcast(cond);
}

s32 PS4_FUNC kernel_pthread_condattr_destroy(pthread_condattr_t* attr) {
    log("pthread_condattr_destroy(attr=*%p)\n", attr);
    return pthread_condattr_destroy(attr);
}

};  // End namespace PS4::OS::Libs::Kernel