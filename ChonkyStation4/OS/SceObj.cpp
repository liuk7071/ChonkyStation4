#include "SceObj.hpp"


namespace PS4::OS {

std::mutex mtx;
u64 next_handle = 0x1000;

SceObj::SceObj() {
    const std::lock_guard<std::mutex> lock(mtx);

    handle = next_handle++;
    objs.push_back(this);
}

// Manually request an handle
u64 requestHandle() {
    return next_handle++;
}

}   // End namespace PS4::OS