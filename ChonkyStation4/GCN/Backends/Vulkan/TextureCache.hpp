#pragma once

#include <Common.hpp>
#include <GCN/TSharp.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <deque>


namespace PS4::GCN::Vulkan {

struct TrackedTexture {
    TSharp  tsharp;
    void* base = nullptr;
    size_t  size = 0;
    u32     width = 0;
    u32     height = 0;
    u64     page = 0;
    u64     page_end = 0;
    bool    dirty = false;
    bool    was_bound = false;
    bool    was_targeted = false;
    bool    is_depth_buffer = false;
    bool    dead = false;
    vk::Format vk_fmt;
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory mem = nullptr;
    vk::raii::ImageView view = nullptr;
    vk::raii::Sampler sampler = nullptr;
    vk::ImageLayout curr_layout = vk::ImageLayout::eUndefined;
    vk::DescriptorImageInfo image_info;
    void transition(vk::ImageLayout new_layout);
};

void getVulkanImageInfoForTSharp(TSharp* tsharp, TrackedTexture** out_info, bool is_depth_buffer = false, vk::Format depth_vk_fmt = vk::Format::eD32Sfloat);

} // End namespace PS4::GCN::Vulkan