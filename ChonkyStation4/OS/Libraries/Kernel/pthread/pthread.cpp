#include "pthread.hpp"
#include <Logger.hpp>
#include <OS/Thread.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

s32 PS4_FUNC kernel_pthread_once(pthread_once_t* once_control, void(*init_routine)()) {
    log("pthread_once(once_control=*%p, init_routine=%p)\n", once_control, init_routine);
    return pthread_once(once_control, init_routine);
}

pthread_t PS4_FUNC kernel_pthread_self() {
    log("pthread_self()\n");
    return pthread_self();
}

s32 PS4_FUNC kernel_pthread_getspecific() {
    log("pthread_getspecific() TODO\n");
    return 0;
}

s32 PS4_FUNC kernel_pthread_setspecific() {
    log("pthread_setspecific() TODO\n");
    return 0;
}

s32 PS4_FUNC kernel_pthread_key_create(pthread_key_t* key, void (*destructor)(void*)) {
    log("pthread_key_create(key=*%p, destructor=%p)\n", key, destructor);
    return pthread_key_create(key, destructor);
}

s32 PS4_FUNC kernel_pthread_attr_init(pthread_attr_t* attr) {
    log("pthread_attr_init(attr=*%p)\n", attr);
    return pthread_attr_init(attr);
}

s32 PS4_FUNC kernel_pthread_attr_get_np(pthread_t pthread, pthread_attr_t* attr) {
    log("pthread_attr_get_np(pthread=*%p, attr=*%p) TODO\n", pthread.p, attr);
    return 0;
}

s32 PS4_FUNC scePthreadAttrGetaffinity(pthread_attr_t* attr, u64* mask) {
    log("scePthreadAttrGetaffinity(attr=*%p, mask=*%p) TODO\n", attr, mask);
    return 0;
}

s32 PS4_FUNC kernel_pthread_attr_getaffinity_np(const pthread_attr_t* attr, size_t cpusetsize, cpu_set_t* cpuset) {
    log("pthread_attr_getaffinity_np(attr=*%p, cpusetsize=%d, cpuset=*%p) TODO\n", attr, cpusetsize, cpuset);
    return 0;
}

s32 PS4_FUNC kernel_pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize) {
    log("pthread_attr_setstacksize(attr=*%p, stacksize=%lld)\n", attr, stacksize);
    return pthread_attr_setstacksize(attr, stacksize);
}

s32 PS4_FUNC kernel_pthread_attr_destroy(pthread_attr_t* attr) {
    log("pthread_attr_destroy(attr=*%p) TODO\n", attr);
    return 0;
}

s32 PS4_FUNC scePthreadCreate(pthread_t* tid, const pthread_attr_t* attr, void* (PS4_FUNC *start)(void*), void* arg, const char* name) {
    log("pthread_create(tid=*%p, attr=*%p, start=%p, arg=%p, name=\"%s\")\n", tid, attr, start, arg, name);
    
    // TODO: attr
    std::string name_str = name ? name : "unnamed";
    auto thread = PS4::OS::Thread::createThread(name_str, (PS4::OS::Thread::ThreadStartFunc)start, arg);
    *tid = thread.getPThread();
    return 0;
}

s32 PS4_FUNC kernel_pthread_detach(pthread_t tid) {
    log("pthread_detach(tid=%p)\n", tid);
    return pthread_detach(tid);
}

};  // End namespace PS4::OS::Libs::Kernel