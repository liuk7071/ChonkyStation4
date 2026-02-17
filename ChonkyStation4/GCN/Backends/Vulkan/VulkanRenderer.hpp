#pragma once

#include <Common.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"


namespace PS4::GCN::Vulkan {

struct Pipeline;

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() : Renderer() {}

    void init() override;
    void draw(const u64 cnt, const void* idx_buf_ptr = nullptr) override;
    void drawIndirect(const u64 cnt, const bool is_indexed, void* draw_args, void* idx_buf_ptr = nullptr, s32 idx_buf_max_size = 0) override;
    void flip(OS::Libs::SceVideoOut::SceVideoOutBuffer* buf) override;

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
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
        VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME,
        VK_EXT_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_EXTENSION_NAME,
        VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME
        //VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
        //VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME
    };

    bool framebuffer_resized = false;

    void recreateSwapChain();
    void advanceSwapchain();

    vk::Extent2D setupRenderingAttachments(Pipeline* pipeline);
};

}   // End namespace PS4::GCN::Vulkan