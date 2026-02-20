#include "SceObj.hpp"


namespace PS4::OS {

std::mutex mtx;
u64 next_handle16   = 0x100;
u64 next_handle     = UINT16_MAX + 1;

SceObj::SceObj(bool handle16bit) {
    const std::lock_guard<std::mutex> lock(mtx);

    if (!handle16bit)
        handle = next_handle++;
    else {
        handle = next_handle16++;
        if (handle > UINT16_MAX)
            Helpers::panic("SceObj: ran out of 16bit handles");
    }

    objs.push_back(this);
}

// Manually request an handle
u64 requestHandle() {
    return next_handle++;
}

}   // End namespace PS4::OS