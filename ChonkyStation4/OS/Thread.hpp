#pragma once

#include <Common.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/Kernel/kernel.hpp>
#include <pthread.h>
#include <deque>


namespace PS4::OS::Thread {

inline thread_local void* guest_tls_ptr;
inline  u64 guest_tls_ptr_offs;
inline  bool initialized = false;

class Thread {
public:
    Thread() {}
    Thread(std::string name) : name(name) {}
    std::string name;

    void* entry;
    void* args;

    pthread_t& getPThread() {
        return thread;
    }
private:
    pthread_t thread;
};

inline std::deque<Thread> threads;

void init();
Thread& createThread(std::string name, void* entry, void* args);
void joinThread(Thread& thread, void** ret);
void joinThread(Thread& thread);
void* threadStart(Thread* thread);

}   // End namespace PS4::OS::Thread