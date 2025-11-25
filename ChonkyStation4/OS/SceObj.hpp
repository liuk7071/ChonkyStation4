#pragma once

#include <Common.hpp>
#include <mutex>


namespace PS4::OS {

inline bool initialized = false;

struct SceObj {
    SceObj();
    u64 handle = 0;
};

inline std::vector<SceObj*> objs;

template<typename T> requires std::is_base_of_v<SceObj, T>
T* make() {
    auto* obj = (T*)objs.emplace_back(new T());
    return obj;
}

template<typename T> requires std::is_base_of_v<SceObj, T>
T* find(u64 handle) {
    for (auto& obj : objs) {
        if (obj->handle == handle)
            return (T*)obj;
    }
    return nullptr;
}

u64 requestHandle();

}   // End namespace PS4::OS