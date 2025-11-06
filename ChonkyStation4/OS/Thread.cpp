#include "Thread.hpp"
#include <Loaders/App.hpp>
#ifdef _WIN32
#include <windows.h>
#endif
#include <thread>


extern "C" unsigned long _tls_index;
extern App g_app;

namespace PS4::OS::Thread {

void init() {
    if (initialized) return;

    // Get the offset of guest_tls_ptr in the host TLS image and save it for later.
    // The pointer of the guest TLS area is stored in the host TLS. We need to access this variable from the patched guest TLS access code.
    // To do that we need to know the offset of the variable in the host TLS image.

    void* tls_ptr = nullptr;

#ifdef _WIN32
    asm volatile(R"(
        movl %1, %%eax
        movq %%gs:0x58, %%rcx
        movq (%%rcx, %%rax, 8), %%rax
        movq %%rax, %0
    )"
    : "=r"(tls_ptr)
    : "r"(_tls_index)
    :    
    );
#else
    Helpers::panic("Unsupported platform\n");
#endif

    guest_tls_ptr_offs = (u8*)&guest_tls_ptr - (u8*)tls_ptr;
    initialized = true;
}

Thread& createThread(std::string name, void* entry, void* args) {
    auto& thread = threads.emplace_back();
    thread.name = name;
    thread.entry = entry;
    thread.args = args;
    pthread_create(&thread.getPThread(), nullptr, (void*(*)(void*))threadStart, &thread);
    return thread;
}

void joinThread(Thread& thread, void** ret) {
    pthread_join(thread.getPThread(), ret);
}

void joinThread(Thread& thread) {
    joinThread(thread, nullptr);
}

void* threadStart(Thread* thread) {
    // Initialize TLS
    // I made a simple TLS test and for some reason thread_local variables seem to start 0x10 bytes earlier than the reported TLS address?
    // Not sure what's happening but I just allocate 0x10 bytes extra to be safe
    auto [tls_image_ptr, tls_image_size] = g_app.getTLSImage();
    guest_tls_ptr = (u8*)std::malloc(tls_image_size + 0x10) + 0x10;
    std::memset((u8*)guest_tls_ptr - 0x10, 0, 0x10);
    std::memcpy(guest_tls_ptr, tls_image_ptr, tls_image_size);

    // Call entry function
    ((void*(*)(void*))thread->entry)(thread->args);
    return nullptr;
}

}   // End namespace PS4::OS::Thread