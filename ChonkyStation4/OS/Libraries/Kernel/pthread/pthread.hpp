#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

struct cpu_set_t;

s32 PS4_FUNC kernel_pthread_once(pthread_once_t* once_control, void(*init_routine)());
void* PS4_FUNC kernel_pthread_self();
const void* PS4_FUNC kernel_pthread_getspecific(u64 key);
s32 PS4_FUNC kernel_pthread_setspecific(u64 key, const void* val);
s32 PS4_FUNC kernel_pthread_key_create(u64* key, void (*destructor)(void*));
s32 PS4_FUNC kernel_pthread_attr_init(pthread_attr_t* attr);
s32 PS4_FUNC kernel_pthread_attr_get_np(void* pthread, pthread_attr_t* attr);
s32 PS4_FUNC kernel_pthread_attr_getaffinity_np(const pthread_attr_t* attr, size_t cpusetsize, cpu_set_t* cpuset);
s32 PS4_FUNC kernel_pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize);
s32 PS4_FUNC kernel_pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate);
s32 PS4_FUNC scePthreadAttrGetaffinity(pthread_attr_t* attr, u64* mask);
s32 PS4_FUNC kernel_pthread_attr_destroy(pthread_attr_t* attr);
s32 PS4_FUNC kernel_pthread_create(void** tid, const pthread_attr_t* attr, void* (PS4_FUNC *start)(void*), void* arg);
s32 PS4_FUNC scePthreadCreate(void** tid, const pthread_attr_t* attr, void* (PS4_FUNC *start)(void*), void* arg, const char* name);
s32 PS4_FUNC scePthreadRename(void* pthread, const char* name);
s32 PS4_FUNC kernel_pthread_detach(void* tid);
s32 PS4_FUNC kernel_pthread_equal(void* tid1, void* tid2);
s32 PS4_FUNC kernel_pthread_yield();
s32 PS4_FUNC kernel_pthread_join(void* pthread, void** ret);
void PS4_FUNC kernel_pthread_exit(void* status);
s32 PS4_FUNC kernel_pthread_getthreadid_np();

};  // End namespace PS4::OS::Libs::Kernel