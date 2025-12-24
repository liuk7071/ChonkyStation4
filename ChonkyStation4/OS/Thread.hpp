#pragma once

#include <Common.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/Kernel/kernel.hpp>
#include <pthread.h>
#include <deque>
#include <unordered_map>


namespace PS4::OS::Thread {

using ThreadStartFunc = PS4_FUNC void* (*)(void* args);

inline  thread_local void* guest_tls_ptr;    // TLS pointer of the main executable's TLS image
inline  thread_local std::unordered_map<u32, void*> tls_map; // Map TLS module ID to pointer
inline  u64 guest_tls_ptr_offs;
inline  bool initialized = false;

class Thread {
public:
    Thread() {}
    Thread(const std::string& name) : name(name) {}
    std::string name;

    ThreadStartFunc entry;
    void* args;
    bool exited = false;

    pthread_t& getPThread() {
        return thread;
    }
private:
    pthread_t thread;
};

inline std::deque<Thread> threads;

void init();
void* getTLSPtr(u32 modid);
Thread& createThread(const std::string& name, ThreadStartFunc entry, void* args);
void joinThread(Thread& thread, void** ret);
void joinThread(Thread& thread);
void* threadStart(Thread* thread);

}   // End namespace PS4::OS::Thread