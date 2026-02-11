#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

s32 PS4_FUNC scePthreadRwlockInit(pthread_rwlock_t* lock, const pthread_rwlockattr_t* attr, const char* name);
s32 PS4_FUNC kernel_pthread_rwlock_rdlock(pthread_rwlock_t* lock);
s32 PS4_FUNC kernel_pthread_rwlock_wrlock(pthread_rwlock_t* lock);
s32 PS4_FUNC kernel_pthread_rwlock_unlock(pthread_rwlock_t* lock);
s32 PS4_FUNC kernel_pthread_rwlockattr_init(pthread_rwlockattr_t* attr);

};  // End namespace PS4::OS::Libs::Kernel