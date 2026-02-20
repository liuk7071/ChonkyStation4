#pragma once

#include <Common.hpp>
#include <mutex>


namespace PS4::OS {

inline bool initialized = false;

struct SceObj {
    SceObj(bool handle16bit);
    u64 handle = 0;
};

inline std::vector<SceObj*> objs;
inline std::mutex obj_mtx;

template<typename T> requires std::is_base_of_v<SceObj, T>
T* make(bool handle16bit = false) {
    auto lk = std::unique_lock<std::mutex>(obj_mtx);
    auto* obj = (T*)objs.emplace_back(new T(handle16bit));
    return obj;
}

inline bool erase(u64 handle) {
    auto lk = std::unique_lock<std::mutex>(obj_mtx);
    for (auto it = objs.begin(); it != objs.end(); it++) {
        if ((*it)->handle == handle) {
            objs.erase(it);
            return true;
        }
    }
    return false;
}

template<typename T> requires std::is_base_of_v<SceObj, T>
T* find(u64 handle) {
    auto lk = std::unique_lock<std::mutex>(obj_mtx);
    for (auto& obj : objs) {
        if (obj->handle == handle)
            return (T*)obj;
    }
    return nullptr;
}

u64 requestHandle();

}   // End namespace PS4::OS