#include "pthread.hpp"
#include <Logger.hpp>
#include <OS/Thread.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

PS4::OS::Thread::Thread& findThread(void* tid) {
    pthread_t* pthread = (pthread_t*)tid;
    PS4::OS::Thread::Thread* curr_thread = nullptr;
    for (auto& thread : PS4::OS::Thread::threads) {
        if (pthread->p == thread.getPThread().p) {
            if (curr_thread == nullptr)
                curr_thread = &thread;
            else {
                if (thread.getPThread().x > curr_thread->getPThread().x)
                    curr_thread = &thread;
            }
        }
    }

    if (!curr_thread)
        Helpers::panic("Could not find pthread");
    return *curr_thread;
}

s32 PS4_FUNC kernel_pthread_once(pthread_once_t* once_control, void(*init_routine)()) {
    log("pthread_once(once_control=*%p, init_routine=%p)\n", once_control, init_routine);
    return pthread_once(once_control, init_routine);
}

void* PS4_FUNC kernel_pthread_self() {
    log("pthread_self()\n");
    pthread_t self = pthread_self();
    for (auto& thread : PS4::OS::Thread::threads) {
        if (self.p == thread.getPThread().p && self.x == thread.getPThread().x) {
            return &thread.getPThread();
        }
    }
    Helpers::panic("pthread_self(): could not find self pthread\n");
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

s32 PS4_FUNC kernel_pthread_attr_get_np(void* pthread, pthread_attr_t* attr) {
    log("pthread_attr_get_np(pthread=*%p, attr=*%p) TODO\n", pthread, attr);
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

s32 PS4_FUNC kernel_pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate) {
    log("pthread_attr_setdetachstate(attr=*%p, detachstate=%d)\n", attr, detachstate);
    return pthread_attr_setdetachstate(attr, detachstate);
}

s32 PS4_FUNC kernel_pthread_attr_destroy(pthread_attr_t* attr) {
    log("pthread_attr_destroy(attr=*%p) TODO\n", attr);
    return 0;
}

s32 PS4_FUNC kernel_pthread_create(void** tid, const pthread_attr_t* attr, void* (PS4_FUNC* start)(void*), void* arg) {
    log("pthread_create(tid=*%p, attr=*%p, start=%p, arg=%p)\n", tid, attr, start, arg);
    scePthreadCreate(tid, attr, start, arg, nullptr);
    return 0;
}

s32 PS4_FUNC scePthreadCreate(void** tid, const pthread_attr_t* attr, void* (PS4_FUNC *start)(void*), void* arg, const char* name) {
    log("scePthreadCreate(tid=*%p, attr=*%p, start=%p, arg=%p, name=\"%s\")\n", tid, attr, start, arg, name);
    // TODO: attr
    std::string name_str = name ? name : "unnamed";
    auto& thread = PS4::OS::Thread::createThread(name_str, (PS4::OS::Thread::ThreadStartFunc)start, arg);
    *tid = (void*)&thread.getPThread();
    return 0;
}

s32 PS4_FUNC kernel_pthread_detach(void* tid) {
    log("pthread_detach(tid=%p)\n", tid);
    return pthread_detach(*(pthread_t*)tid);
}

s32 PS4_FUNC kernel_pthread_equal(void* tid1, void* tid2) {
    log("pthread_equal(tid1=%p, tid2=%p)\n", tid1, tid2);
    return tid1 == tid2;
}

s32 PS4_FUNC kernel_pthread_yield() {
    log("pthread_yield()\n");
    //std::this_thread::yield();
    return 0;
}

s32 PS4_FUNC kernel_pthread_join(void* pthread, void** ret) {
    log("pthread_join(pthread=*%p, ret=*%p)\n", pthread, ret);
    auto thread = findThread(pthread);
    OS::Thread::joinThread(thread, ret);
    return SCE_OK;
}

void PS4_FUNC kernel_pthread_exit(void* status) {
    log("pthread_exit(status=%p)\n", status);
    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    pthread_exit(status);
}

};  // End namespace PS4::OS::Libs::Kernel