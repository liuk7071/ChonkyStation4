#include "Semaphore.hpp"
#include <Logger.hpp>
#include <ErrorCodes.hpp>
#include <OS/Libraries/Kernel/Kernel.hpp>


namespace PS4::OS::Libs::Kernel {

MAKE_LOG_FUNCTION(log, lib_kernel_sema);

void Semaphore::signal(s32 count, bool is_from_cancel) {
    if (!is_from_cancel) cancelled = false;
    
    while (count--) {
        counter++;
        std_sema->release();
    }
}

bool Semaphore::wait(s32 count, u32 timeout, bool& was_cancelled) {
    auto lk = std::unique_lock<std::mutex>(mtx);
    was_cancelled = false;

    if (!timeout) {
        waiters++;
        while (count--) {
            std_sema->acquire();
            counter--;
        }

        waiters--;
    }
    else {
        using namespace std::chrono;
        const auto start = steady_clock::now();
        const auto deadline = start + microseconds(timeout);
        s32 decremented_count = 0;  // Count to re-add in case we timeout

        waiters++;
        while (count--) {
            if (!std_sema->try_acquire_until(deadline)) {
                // We timed out, restore counter and return error
                signal(decremented_count);
                waiters--;
                return false;
            }
  
            counter--;
            decremented_count++;
        }
        waiters--;
    }

    if (cancelled) {
        was_cancelled = true;
        woke_from_cancel++;
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

int Semaphore::cancel(s32 new_count) {
    woke_from_cancel = 0;
    cancelled = true;

    while (waiters)
        signal(1, true);

    counter = new_count >= 0 ? new_count : init_count;
    return woke_from_cancel;
}

s32 PS4_FUNC sceKernelCreateSema(SceKernelSema* sem, const char* name, u32 attr, s32 init_count, s32 max_count, const SceKernelSemaOptParam* opt_param) {
    log("sceKernelCreateSema(sem=*%p, name=\"%s\", attr=0x%x, init_count=%d, max_count=%d, opt_param=*%p)\n", sem, name ? name : "unnamed (returning error)", attr, init_count, max_count, opt_param);
    
    if (!name) return SCE_KERNEL_ERROR_EINVAL;

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

    bool was_cancelled = false;
    if (!sem->wait(count, timeout ? *timeout : 0, was_cancelled))
        return SCE_KERNEL_ERROR_ETIMEDOUT;
    
    if (was_cancelled)
        return SCE_KERNEL_ERROR_ECANCELED;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelPollSema(SceKernelSema sem, s32 count) {
    log("sceKernelPollSema(sem=%p, count=%d)\n", sem, count);

    if (!sem->poll(count))
        return SCE_KERNEL_ERROR_EBUSY;
    return SCE_OK;
}

s32 PS4_FUNC sceKernelCancelSema(SceKernelSema sem, s32 set_count, s32* n_released_threads) {
    log("sceKernelCancelSema(sem=%p, set_count=%d, n_released_threads=*%p)\n");

    auto count = sem->cancel(set_count);
    if (n_released_threads)
        *n_released_threads = count;
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

    bool was_cancelled = false;
    (*sem)->wait(1, 0, was_cancelled);

    if (was_cancelled)
        return POSIX_ECANCELED;
    return 0;
}

s32 PS4_FUNC kernel_sem_timedwait(SceKernelSema* sem, const SceKernelTimespec* time) {
    log("sem_timedwait(sem=*%p)\n", sem);

    using namespace std::chrono;
    //auto us = duration_cast<microseconds>(seconds(time->tv_sec) + nanoseconds(time->tv_nsec)).count();
    auto us = 1000;

    bool was_cancelled = false;
    if (!(*sem)->wait(1, us, was_cancelled)) {
        *Kernel::kernel_error() = POSIX_ETIMEDOUT;
        return -1;
    }

    if (was_cancelled)
        return POSIX_ECANCELED;
    return 0;
}

s32 PS4_FUNC kernel_sem_getvalue(SceKernelSema* sem, s32* val) {
    log("sem_getvalue(sem=*%p, val=*%p)\n", sem, val);

    *val = (*sem)->counter;
    return 0;
}

};  // End namespace PS4::OS::Libs::Kernel