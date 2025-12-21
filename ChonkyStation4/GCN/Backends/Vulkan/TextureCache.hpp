#pragma once

#include <Common.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <deque>


struct TSharp;

namespace PS4::GCN::Vulkan {

void getVulkanImageInfoForTSharp(TSharp* tsharp, vk::DescriptorImageInfo** out_info);
std::pair<vk::Format, size_t> getTexFormatAndSize(u32 dfmt, u32 nfmt);

} // End namespace PS4::GCN::Vulkan