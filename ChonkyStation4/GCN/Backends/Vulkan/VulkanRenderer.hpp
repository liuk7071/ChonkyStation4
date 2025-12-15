#pragma once

#include <Common.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace PS4::GCN::Vulkan {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() : Renderer() {}

    void init() override;
    void draw(const u64 cnt, const void* idx_buf_ptr = nullptr) override;
    void flip() override;

private:
    u64 frame_count = 0;
    double last_time = 0.0;

    vk::raii::Context                context;
    vk::raii::Instance               instance           = nullptr;
    vk::raii::DebugUtilsMessengerEXT debug_messenger    = nullptr;
    vk::raii::SurfaceKHR             surface            = nullptr;
    u32                              queue_index        = ~0;
    vk::raii::SwapchainKHR           swapchain          = nullptr;
    std::vector<vk::Image>           swapchain_images;
    std::vector<vk::raii::ImageView> swapchain_image_views;

    std::vector<vk::raii::CommandBuffer> cmd_bufs;

    vk::raii::Semaphore present_sema = nullptr;
    vk::raii::Semaphore render_sema = nullptr;
    vk::raii::Fence draw_fence = nullptr;
    u32 sema_idx = 0;
    u32 curr_frame = 0;
    u32 current_swapchain_image_idx = 0;

    std::vector<const char*> required_device_exts = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
    };

    bool framebuffer_resized = false;

    void recreateSwapChain();
    void transitionImageLayoutForSwapchain(
        u32                     img_idx,
        vk::ImageLayout         old_layout,
        vk::ImageLayout         new_layout,
        vk::AccessFlags2        src_access_mask,
        vk::AccessFlags2        dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask);
    void advanceSwapchain();
};

}   // End namespace PS4::GCN::Vulkan