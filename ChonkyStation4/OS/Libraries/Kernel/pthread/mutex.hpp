#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {
        
#ifdef _WIN32

#define MUTEX_IMPL pthread_mutex_t
#define MUTEXATTR_IMPL pthread_mutexattr_t
    
#else
    
struct MutexWrapper {
    char padding[64];
    pthread_mutex_t mutex;
};

struct MutexAttrWrapper {
    char padding[64];
    pthread_mutexattr_t attr;
};

#define MUTEX_IMPL MutexWrapper*
#define MUTEXATTR_IMPL MutexAttrWrapper*

#endif

s32 PS4_FUNC kernel_pthread_mutex_lock(MUTEX_IMPL* mutex);
s32 PS4_FUNC kernel_pthread_mutex_trylock(MUTEX_IMPL* mutex);
s32 PS4_FUNC scePthreadMutexTimedlock(MUTEX_IMPL* mutex, u64 us);
s32 PS4_FUNC kernel_pthread_mutex_unlock(MUTEX_IMPL* mutex);
s32 PS4_FUNC kernel_pthread_mutexattr_init(MUTEXATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_mutexattr_destroy(MUTEXATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_mutexattr_settype(MUTEXATTR_IMPL* attr, int kind);
s32 PS4_FUNC kernel_pthread_mutex_init(MUTEX_IMPL* mutex, const MUTEXATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_mutex_destroy(MUTEX_IMPL* mutex);

};  // End namespace PS4::OS::Libs::Kernel