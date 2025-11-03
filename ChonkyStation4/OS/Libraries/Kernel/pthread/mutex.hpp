#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

u32 PS4_FUNC kernel_pthread_mutex_lock(pthread_mutex_t* mutex);
u32 PS4_FUNC kernel_pthread_mutex_unlock(pthread_mutex_t* mutex);

};  // End namespace PS4::OS::Libs::Kernel