#pragma once

#include <Common.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <deque>


struct TSharp;

namespace PS4::GCN::Vulkan {

void initTextureCache();
void getVulkanImageInfoForTSharp(TSharp* tsharp, vk::DescriptorImageInfo** out_info);

} // End namespace PS4::GCN::Vulkan