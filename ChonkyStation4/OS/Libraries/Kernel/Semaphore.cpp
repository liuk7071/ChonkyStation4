#include "Semaphore.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_sema);

void Semaphore::signal(s32 count) {
    while (count--) {
        counter++;
        std_sema->release();
    }
}

bool Semaphore::wait(s32 count, u32 timeout) {
    auto lk = std::unique_lock<std::mutex>(mtx);
    
    if (!timeout) {
        while (count--) {
            std_sema->acquire();
            counter--;
        }
    }
    else {
        using namespace std::chrono;
        const auto start = steady_clock::now();
        const auto deadline = start + microseconds(timeout);
        s32 decremented_count = 0;  // Count to re-add in case we timeout

        while (count--) {
            if (!std_sema->try_acquire_until(deadline)) {
                // We timed out, restore counter and return error
                signal(decremented_count);
                return false;
            }
  
            counter--;
            decremented_count++;
        }
    }

    return true;
}

bool Semaphore::poll(s32 count) {
    auto lk = std::unique_lock<std::mutex>(mtx);

    if (counter >= count) {
        while (count--) {
            std_sema->acquire();
            counter--;
        }
        return true;
    }
    return false;
}

s32 PS4_FUNC sceKernelCreateSema(SceKernelSema* sem, const char* name, u32 attr, s32 init_count, s32 max_count, const SceKernelSemaOptParam* opt_param) {
    log("sceKernelCreateSema(sem=*%p, name=\"%s\", attr=0x%x, init_count=%d, max_count=%d, opt_param=*%p)\n", sem, name, attr, init_count, max_count, opt_param);

    *sem = new Semaphore(init_count, max_count);
    (*sem)->name = name;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelSignalSema(SceKernelSema sem, s32 count) {
    log("sceKernelSignalSema(sem=%p, count=%d)\n", sem, count);

    if (sem == nullptr) {
        printf("WARNING: semaphore is nullptr\n");
        return SCE_KERNEL_ERROR_ESRCH;
    }

    sem->signal(count);
    return SCE_OK;
}

s32 PS4_FUNC sceKernelWaitSema(SceKernelSema sem, s32 count, u32* timeout) {
    log("sceKernelWaitSema(sem=%p, count=%d, timeout=*%p)\n", sem, count, timeout);

    if (!sem->wait(count, timeout ? *timeout : 0))
        return SCE_KERNEL_ERROR_ETIMEDOUT;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelPollSema(SceKernelSema sem, s32 count) {
    log("sceKernelPollSema(sem=%p, count=%d)\n", sem, count);

    if (!sem->poll(count))
        return SCE_KERNEL_ERROR_EBUSY;
    return SCE_OK;
}

s32 PS4_FUNC kernel_sem_init(SceKernelSema* sem, s32 pshared, u32 value) {
    log("sem_init(sem=*%p, pshared=%d, value=%d)\n", sem, pshared, value);

    *sem = new Semaphore(value, INT32_MAX);
    return 0;
}

s32 PS4_FUNC kernel_sem_post(SceKernelSema* sem) {
    log("sem_post(sem=*%p)\n", sem);

    (*sem)->signal(1);
    return 0;
}

s32 PS4_FUNC kernel_sem_wait(SceKernelSema* sem) {
    log("sem_wait(sem=*%p)\n", sem);

    (*sem)->wait(1, 0);
    return 0;
}

};  // End namespace PS4::OS::Libs::Kernel