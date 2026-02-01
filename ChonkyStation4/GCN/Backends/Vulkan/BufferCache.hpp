#pragma once

#include <Common.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"
#include <tuple>
#include <functional>


namespace PS4::GCN::Vulkan::Cache {

inline size_t page_size = 0;
inline u32    page_bits = 0;

void init();
std::tuple<vk::Buffer, size_t, bool> getBuffer(void* base, size_t size);
std::pair<vk::Buffer, void*> getMappedBufferForFrame(size_t size);
void track(void* base, size_t size, std::function<void(uptr)> callback);
bool resetDirty(void* base, size_t size);
bool isDirty(void* base, size_t size);
void clear();

}   // End namespace PS4::GCN::Vulkan::Cache