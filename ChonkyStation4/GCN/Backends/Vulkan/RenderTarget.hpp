#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace PS4::GCN {

struct ColorTarget;
struct DepthTarget;

}   // End namespace PS4::GCN

namespace PS4::GCN::Vulkan {

struct TrackedTexture;

}   // End namespace PS4::GCN::Vulkan

namespace PS4::GCN::Vulkan::RenderTarget {

struct Attachment {
    vk::RenderingAttachmentInfo vk_attachment;
    TrackedTexture* tex;
    bool has_feedback_loop;
};

Attachment getVulkanAttachmentForColorTarget(ColorTarget* rt, bool* save);
Attachment getVulkanAttachmentForDepthTarget(DepthTarget* depth, bool* save);
void reset();

}   // End namespace PS4::GCN::Vulkan::RenderTarget