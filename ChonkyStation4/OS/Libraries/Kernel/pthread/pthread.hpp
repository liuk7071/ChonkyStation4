#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

u32 PS4_FUNC kernel_pthread_once(pthread_once_t* once_control, void(*init_routine)());
u32 PS4_FUNC kernel_pthread_getspecific();
u32 PS4_FUNC kernel_pthread_setspecific();
u32 PS4_FUNC kernel_pthread_key_create(pthread_key_t * key, void (*destructor)(void*));

};  // End namespace PS4::OS::Libs::Kernel