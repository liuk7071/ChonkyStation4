#pragma once

#include <Common.hpp>
#include <pthread.h>


namespace PS4::OS::Libs::Kernel {

struct cpu_set_t;

#ifdef _WIN32

#define PTHREADATTR_IMPL pthread_attr_t

#else

struct PthreadAttrWrapper {
    char padding[64];
    pthread_attr_t attr;
};

#define PTHREADATTR_IMPL PthreadAttrWrapper*

#endif

s32 PS4_FUNC kernel_pthread_once(pthread_once_t* once_control, void(*init_routine)());
void* PS4_FUNC kernel_pthread_self();
const void* PS4_FUNC kernel_pthread_getspecific(s32 key);
s32 PS4_FUNC kernel_pthread_setspecific(s32 key, const void* val);
s32 PS4_FUNC kernel_pthread_key_create(s32* key, void (*destructor)(void*));
s32 PS4_FUNC kernel_pthread_attr_init(PTHREADATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_attr_get_np(void* pthread, PTHREADATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_attr_getaffinity_np(const PTHREADATTR_IMPL* attr, size_t cpusetsize, cpu_set_t* cpuset);
s32 PS4_FUNC kernel_pthread_attr_getstack(PTHREADATTR_IMPL* attr, void** stack_addr, size_t* stack_size);
s32 PS4_FUNC kernel_pthread_attr_setstacksize(PTHREADATTR_IMPL* attr, size_t stacksize);
s32 PS4_FUNC kernel_pthread_attr_setdetachstate(PTHREADATTR_IMPL* attr, int detachstate);
s32 PS4_FUNC scePthreadAttrGetaffinity(PTHREADATTR_IMPL* attr, u64* mask);
s32 PS4_FUNC kernel_pthread_attr_destroy(PTHREADATTR_IMPL* attr);
s32 PS4_FUNC kernel_pthread_create(void** tid, const PTHREADATTR_IMPL* attr, void* (PS4_FUNC *start)(void*), void* arg);
s32 PS4_FUNC scePthreadCreate(void** tid, const PTHREADATTR_IMPL* attr, void* (PS4_FUNC *start)(void*), void* arg, const char* name);
s32 PS4_FUNC scePthreadRename(void* pthread, const char* name);
s32 PS4_FUNC kernel_pthread_detach(void* tid);
s32 PS4_FUNC kernel_pthread_equal(void* tid1, void* tid2);
s32 PS4_FUNC kernel_pthread_yield();
s32 PS4_FUNC kernel_pthread_join(void* pthread, void** ret);
void PS4_FUNC kernel_pthread_exit(void* status);
s32 PS4_FUNC kernel_pthread_getthreadid_np();

};  // End namespace PS4::OS::Libs::Kernel