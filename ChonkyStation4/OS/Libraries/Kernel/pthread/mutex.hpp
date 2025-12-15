#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

s32 PS4_FUNC kernel_pthread_mutex_lock(pthread_mutex_t* mutex);
s32 PS4_FUNC kernel_pthread_mutex_trylock(pthread_mutex_t* mutex);
s32 PS4_FUNC kernel_pthread_mutex_unlock(pthread_mutex_t* mutex);
s32 PS4_FUNC kernel_pthread_mutexattr_init(pthread_mutexattr_t* attr);
s32 PS4_FUNC kernel_pthread_mutexattr_destroy(pthread_mutexattr_t* attr);
s32 PS4_FUNC kernel_pthread_mutexattr_settype(pthread_mutexattr_t* attr, int kind);
s32 PS4_FUNC kernel_pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
s32 PS4_FUNC kernel_pthread_mutex_destroy(pthread_mutex_t* mutex);

};  // End namespace PS4::OS::Libs::Kernel