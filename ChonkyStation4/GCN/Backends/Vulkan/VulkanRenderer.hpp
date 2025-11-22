#pragma once

#include <Common.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace PS4::GCN {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() : Renderer() {}

    void init() override;
    void draw(u64 cnt) override;
    void flip() override;

private:
    u64 frame_count = 0;
    double last_time = 0.0;

    vk::raii::Context                context;
    vk::raii::Instance               instance           = nullptr;
    vk::raii::DebugUtilsMessengerEXT debug_messenger    = nullptr;
    vk::raii::SurfaceKHR             surface            = nullptr;
    vk::raii::PhysicalDevice         physical_device    = nullptr;
    vk::raii::Device                 device             = nullptr;
    u32                              queue_index        = ~0;
    vk::raii::Queue                  queue              = nullptr;
    vk::raii::SwapchainKHR           swapchain          = nullptr;
    std::vector<vk::Image>           swapchain_images;
    vk::SurfaceFormatKHR             swapchain_surface_format;
    vk::Extent2D                     swapchain_extent;
    std::vector<vk::raii::ImageView> swapchain_image_views;

    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline graphics_pipeline = nullptr;

    vk::raii::CommandPool cmd_pool = nullptr;
    std::vector<vk::raii::CommandBuffer> cmd_bufs;

    vk::raii::Semaphore present_sema = nullptr;
    vk::raii::Semaphore render_sema = nullptr;
    vk::raii::Fence draw_fence = nullptr;
    u32 sema_idx = 0;
    u32 curr_frame = 0;
    u32 current_swapchain_image_idx = 0;

    std::vector<vk::raii::Buffer> vtx_bufs;
    std::vector<vk::raii::DeviceMemory> vtx_bufs_mem;

    std::vector<const char*> required_device_exts = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
    };

    bool framebuffer_resized = false;

    vk::raii::ShaderModule createShaderModule(const std::vector<u32>& code);
    void recreateSwapChain();
    u32 findMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties);
    void VulkanRenderer::transitionImageLayout(
        u32                     img_idx,
        vk::ImageLayout         old_layout,
        vk::ImageLayout         new_layout,
        vk::AccessFlags2        src_access_mask,
        vk::AccessFlags2        dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask);
    void advanceSwapchain();

    vk::Format getVtxBufferFormat(u32 n_elements, u32 type);
};

}   // End namespace PS4::GCN