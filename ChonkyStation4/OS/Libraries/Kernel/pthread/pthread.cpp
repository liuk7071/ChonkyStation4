#include "pthread.hpp"
#include <Logger.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel);

u32 PS4_FUNC kernel_pthread_once(pthread_once_t* once_control, void(*init_routine)()) {
    log("pthread_once(once_control=%p, init_routine=%p)\n", once_control, init_routine);
    return pthread_once(once_control, init_routine);
}

u32 PS4_FUNC kernel_pthread_getspecific() {
    log("pthread_getspecific() TODO\n");
    return 0;
}

u32 PS4_FUNC kernel_pthread_setspecific() {
    log("pthread_setspecific() TODO\n");
    return 0;
}

u32 PS4_FUNC kernel_pthread_key_create(pthread_key_t * key, void (*destructor)(void*)) {
    log("pthread_key_create(key=%p, destructor=%p)\n", key, destructor);
    return pthread_key_create(key, destructor);
}

};  // End namespace PS4::OS::Libs::Kernel