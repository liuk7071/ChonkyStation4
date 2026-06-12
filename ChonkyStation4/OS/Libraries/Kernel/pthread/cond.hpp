#pragma once

#include <Common.hpp>
#include <pthread.h>
#include <OS/Libraries/Kernel/pthread/mutex.hpp>


namespace PS4::OS::Libs::Kernel {

#ifdef _WIN32

#define COND_IMPL pthread_cond_t
#define CONDATTR_IMPL pthread_condattr_t

#else

struct CondWrapper {
    char padding[64];
    pthread_cond_t cond;
};

struct CondAttrWrapper {
    char padding[64];
    pthread_condattr_t attr;
};

#define COND_IMPL CondWrapper*
#define CONDATTR_IMPL CondAttrWrapper*

#endif

s32 PS4_FUNC kernel_pthread_condattr_init(CONDATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_cond_init(COND_IMPL* cond, const CONDATTR_IMPL* attr);
s32 PS4_FUNC scePthreadCondInit(COND_IMPL* cond, const CONDATTR_IMPL* attr, const char* name);
s32 PS4_FUNC kernel_pthread_cond_wait(COND_IMPL* cond, MUTEX_IMPL* mutex);
s32 PS4_FUNC kernel_pthread_cond_timedwait(COND_IMPL* cond, MUTEX_IMPL* mutex, const SceKernelTimespec* abstime);
s32 PS4_FUNC scePthreadCondTimedwait(COND_IMPL* cond, MUTEX_IMPL* mutex, u64 us);
s32 PS4_FUNC kernel_pthread_cond_signal(COND_IMPL* cond);
s32 PS4_FUNC kernel_pthread_cond_broadcast(COND_IMPL* cond);
s32 PS4_FUNC kernel_pthread_condattr_destroy(CONDATTR_IMPL* attr);

};  // End namespace PS4::OS::Libs::Kernel