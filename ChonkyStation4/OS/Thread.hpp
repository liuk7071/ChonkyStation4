#pragma once

#include <Common.hpp>
#include <Loaders/Module.hpp>
#include <OS/Libraries/Kernel/Kernel.hpp>
#include <pthread.h>
#include <deque>
#include <unordered_map>


namespace PS4::OS::Thread {

using ThreadStartFunc = PS4_FUNC void* (*)(void* args);

inline  thread_local void* guest_tls_ptr;    // TLS pointer of the main executable's TLS image
inline  thread_local std::unordered_map<u32, void*> tls_map; // Map TLS module ID to pointer
inline  u64 guest_tls_ptr_offs;
inline  bool initialized = false;

static s32 next_tid = 1;
class Thread {
public:
    Thread() {}
    Thread(const std::string& name) : name(name) {
        tid = next_tid++;
    }
    std::string name;

    ThreadStartFunc entry;
    void* args;
    bool exited = false;
    void* ret_val = nullptr;

    pthread_t& getPThread() { return thread; }
    s32        getTID()     { return tid;    }
private:
    pthread_t thread;
    s32 tid;
};

inline std::deque<Thread> threads;

void init();
void* getTLSPtr(u32 modid);
Thread& createThread(const std::string& name, ThreadStartFunc entry, void* args);
void joinThread(Thread& thread, void** ret);
void joinThread(Thread& thread);
void* threadStart(Thread* thread);

}   // End namespace PS4::OS::Thread