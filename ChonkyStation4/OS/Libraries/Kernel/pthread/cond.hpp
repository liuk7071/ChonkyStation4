#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

s32 PS4_FUNC kernel_pthread_condattr_init(pthread_condattr_t* attr);
s32 PS4_FUNC kernel_pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr);
s32 PS4_FUNC scePthreadCondInit(pthread_cond_t* cond, const pthread_condattr_t* attr, const char* name);
s32 PS4_FUNC kernel_pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);
s32 PS4_FUNC kernel_pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const SceKernelTimespec* abstime);
s32 PS4_FUNC scePthreadCondTimedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, u64 us);
s32 PS4_FUNC kernel_pthread_cond_signal(pthread_cond_t* cond);
s32 PS4_FUNC kernel_pthread_cond_broadcast(pthread_cond_t* cond);
s32 PS4_FUNC kernel_pthread_condattr_destroy(pthread_condattr_t* attr);

};  // End namespace PS4::OS::Libs::Kernel