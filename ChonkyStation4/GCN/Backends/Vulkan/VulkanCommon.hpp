#pragma once

#include <Common.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GCN/DataFormats.hpp>


namespace PS4::GCN::Vulkan {

inline vk::raii::PhysicalDevice             physical_device = nullptr;
inline vk::raii::Device                     device = nullptr;
inline vk::raii::Queue                      queue = nullptr;
inline vk::raii::CommandPool                cmd_pool = nullptr;
inline vk::SurfaceFormatKHR                 swapchain_surface_format;
inline vk::Extent2D                         swapchain_extent;

vk::raii::CommandBuffer beginCommands();
void endCommands(vk::raii::CommandBuffer& cmd_buffer);
void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
u32 findMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties);
std::pair<vk::Format, size_t> getBufFormatAndSize(u32 dfmt, u32 nfmt);

}   // End namespace PS4::GCN::Vulkan