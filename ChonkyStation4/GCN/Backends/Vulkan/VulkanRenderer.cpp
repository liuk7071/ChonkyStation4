#pragma once

#define NOMINMAX
#include "VulkanRenderer.hpp"
#include <Logger.hpp>
#include <Loaders/App.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/PipelineCache.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <SDL_vulkan.h>
#include <OS/Libraries/ScePad/ScePad.hpp>


extern App g_app;

namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

constexpr bool enable_validation_layers = false;

const std::vector<char const*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR unsigned int VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    if ((vk::DebugUtilsMessageSeverityFlagBitsEXT)severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || (vk::DebugUtilsMessageSeverityFlagBitsEXT)severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        std::cerr << "validation layer: type " << to_string((vk::DebugUtilsMessageTypeFlagsEXT)type) << " msg: " << pCallbackData->pMessage << std::endl;
    }

    return vk::False;
}

static u32 chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surface_capabilities) {
    auto min_image_count = std::max(3u, surface_capabilities.minImageCount);
    if ((0 < surface_capabilities.maxImageCount) && (surface_capabilities.maxImageCount < min_image_count)) {
        min_image_count = surface_capabilities.maxImageCount;
    }
    return min_image_count;
}

static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& available_formats) {
    Helpers::debugAssert(!available_formats.empty(), "chooseSwapSurfaceFormat: no available formats");
    const auto format_it = std::ranges::find_if(
        available_formats,
        [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
    return format_it != available_formats.end() ? *format_it : available_formats[0];
}

static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& available_present_modes) {
    return vk::PresentModeKHR::eImmediate;
}

static vk::Extent2D chooseSwapExtent(SDL_Window* window, const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != 0xFFFFFFFF) {
        return capabilities.currentExtent;
    }

    int width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    return {
        std::clamp<u32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<u32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}

void VulkanRenderer::recreateSwapChain() {
    device.waitIdle();
    swapchain_image_views.clear();
    swapchain = nullptr;
    auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
    swapchain_extent = chooseSwapExtent(window, surface_capabilities);
    swapchain_surface_format = chooseSwapSurfaceFormat(physical_device.getSurfaceFormatsKHR(*surface));
    vk::SwapchainCreateInfoKHR swapchain_create_info = {
        .surface = *surface,
        .minImageCount = chooseSwapMinImageCount(surface_capabilities),
        .imageFormat = swapchain_surface_format.format,
        .imageColorSpace = swapchain_surface_format.colorSpace,
        .imageExtent = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = chooseSwapPresentMode(physical_device.getSurfacePresentModesKHR(*surface)),
        .clipped = true
    };

    swapchain = vk::raii::SwapchainKHR(device, swapchain_create_info);
    swapchain_images = swapchain.getImages();
    vk::ImageViewCreateInfo image_view_create_info = { .viewType = vk::ImageViewType::e2D, .format = swapchain_surface_format.format, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    for (auto image : swapchain_images) {
        image_view_create_info.image = image;
        swapchain_image_views.emplace_back(device, image_view_create_info);
    }
}

void VulkanRenderer::transitionImageLayoutForSwapchain(u32 img_idx, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask, vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask) {
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchain_images[img_idx],
        .subresourceRange = {
               .aspectMask = vk::ImageAspectFlagBits::eColor,
               .baseMipLevel = 0,
               .levelCount = 1,
               .baseArrayLayer = 0,
               .layerCount = 1 }
    };
    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    cmd_bufs[0].pipelineBarrier2(dependency_info);
}

void VulkanRenderer::advanceSwapchain() {
    while (true) {
        auto [result, image_idx] = swapchain.acquireNextImage(UINT64_MAX, *present_sema, nullptr);
        current_swapchain_image_idx = image_idx;

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapChain();
            continue;   // Retry to acquire next image
        }

        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            Helpers::panic("Vulkan: failed to acquire swapchain image!");
        }

        break;
    }
}

void VulkanRenderer::init() {

    // ---- Create the SDL window ----

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
        Helpers::panic("Failed to initialize SDL\n");

    window = SDL_CreateWindow(std::format("ChonkyStation4 | {}", g_app.name).c_str(), 100, 100, 1920, 1080, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    if (window == nullptr) {
        Helpers::panic("Failed to create SDL window: %s\n", SDL_GetError());
    }
    
    // ---- Setup Vulkan ----
    
    constexpr vk::ApplicationInfo app_info = {
        .pApplicationName   = "ChonkyStation4",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName        = "No Engine",
        .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion         = vk::ApiVersion13
    };

    // Get the required layers
    std::vector<char const*> required_layers;
    if (enable_validation_layers) {
        required_layers.assign(validation_layers.begin(), validation_layers.end());
    }

    // Check if the required layers are supported by the Vulkan implementation
    auto layer_properties = context.enumerateInstanceLayerProperties();
    for (auto const& required_layer : required_layers) {
        if (std::ranges::none_of(layer_properties, [required_layer](auto const& layerProperty) { return strcmp(layerProperty.layerName, required_layer) == 0; })) {
            Helpers::panic("Required layer not supported: %s", required_layer);
        }
    }

    // Get the required extensions
    u32 n_exts = 0;
    std::vector<const char*> required_exts;
    SDL_Vulkan_GetInstanceExtensions(window, &n_exts, nullptr);
    required_exts.resize(n_exts);
    SDL_Vulkan_GetInstanceExtensions(window, &n_exts, required_exts.data());
    
    if (enable_validation_layers)
        required_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // Check if the required extensions are supported by the Vulkan implementation
    auto extension_properties = context.enumerateInstanceExtensionProperties();
    for (auto const& required_exts : required_exts) {
        if (std::ranges::none_of(extension_properties, [required_exts](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, required_exts) == 0; })) {
            Helpers::panic("Required extension not supported: %s", required_exts);
        }
    }

    vk::InstanceCreateInfo create_info = {
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = static_cast<u32>(required_layers.size()),
        .ppEnabledLayerNames     = required_layers.data(),
        .enabledExtensionCount   = static_cast<u32>(required_exts.size()),
        .ppEnabledExtensionNames = required_exts.data()
    };
    instance = vk::raii::Instance(context, create_info);
    
    // Setup the debug messenger if validation layers are enabled
    if (enable_validation_layers) {
        vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT     message_type_flags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        vk::DebugUtilsMessengerCreateInfoEXT  debug_utils_messenger_create_info_ext = {
            .messageSeverity = severity_flags,
            .messageType = message_type_flags,
            .pfnUserCallback = &debugCallback
        };
        debug_messenger = instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info_ext);
    }

    // Create Vulkan window surface
    VkSurfaceKHR _surface;
    SDL_Vulkan_CreateSurface(window, *instance, &_surface);
    surface = vk::raii::SurfaceKHR(instance, _surface);

    // Choose a physical device
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    const auto dev_iter = std::ranges::find_if(devices, [&](auto const &device) {
        // Check if the device supports the Vulkan 1.3 API version
        bool supports_vulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;

        // Check if any of the queue families support graphics operations
        auto queue_families = device.getQueueFamilyProperties();
        bool supports_graphics =
            std::ranges::any_of(queue_families, [](auto const &qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

        // Check if all required device extensions are available
        auto available_device_exts = device.enumerateDeviceExtensionProperties();
        bool supports_all_required_exts = std::ranges::all_of(required_device_exts,
        [&available_device_exts](auto const &required_device_exts) {
            return std::ranges::any_of(available_device_exts,
            [required_device_exts](auto const &availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, required_device_exts) == 0; });
        });

        auto features                 = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supports_all_required_features =    features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters
                                        && features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering
                                        && features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        return supports_vulkan1_3 && supports_graphics && supports_all_required_exts && supports_all_required_features;
    });

    if (dev_iter != devices.end()) {
        physical_device = *dev_iter;
    }
    else Helpers::panic("Vulkan: failed to find a suitable GPU!");

    // Get the first index into queue_family_properties which supports both graphics and present
    std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();
    for (u32 qfp_idx = 0; qfp_idx < queue_family_properties.size(); qfp_idx++) {
        if ((queue_family_properties[qfp_idx].queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(qfp_idx, *surface)) {
            // Found a queue family that supports both graphics and present
            queue_index = qfp_idx;
            break;
        }
    }
    if (queue_index == ~0) Helpers::panic("Could not find a queue family for graphics and presentation");

    // Query for required features (Vulkan 1.1 and 1.3)
    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain = {
        {},                                                             // vk::PhysicalDeviceFeatures2
        { .shaderDrawParameters = true                           },     // vk::PhysicalDeviceVulkan11Features
        { .synchronization2     = true, .dynamicRendering = true },     // vk::PhysicalDeviceVulkan13Features
        { .extendedDynamicState = true                           }      // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    };

    // Create a (logical) Device
    float queue_prio = 0.0f;
    vk::DeviceQueueCreateInfo device_queue_crerate_info = { .queueFamilyIndex = queue_index, .queueCount = 1, .pQueuePriorities = &queue_prio };
    vk::DeviceCreateInfo device_create_info = {
        .pNext                   = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
         .queueCreateInfoCount    = 1,
         .pQueueCreateInfos       = &device_queue_crerate_info,
         .enabledExtensionCount   = static_cast<u32>(required_device_exts.size()),
         .ppEnabledExtensionNames = required_device_exts.data()
    };

    device = vk::raii::Device(physical_device, device_create_info);
    queue  = vk::raii::Queue(device, queue_index, 0);

    // Create swapchain
    auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
    swapchain_extent          = chooseSwapExtent(window, surface_capabilities);
    swapchain_surface_format   = chooseSwapSurfaceFormat(physical_device.getSurfaceFormatsKHR(*surface));
    vk::SwapchainCreateInfoKHR swapchain_create_info = {
        .surface          = *surface,
        .minImageCount    = chooseSwapMinImageCount(surface_capabilities),
        .imageFormat      = swapchain_surface_format.format,
        .imageColorSpace  = swapchain_surface_format.colorSpace,
        .imageExtent      = swapchain_extent,
        .imageArrayLayers = 1,
        .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform     = surface_capabilities.currentTransform,
        .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode      = chooseSwapPresentMode(physical_device.getSurfacePresentModesKHR(*surface)),
        .clipped          = true
    };

    swapchain        = vk::raii::SwapchainKHR(device, swapchain_create_info);
    swapchain_images = swapchain.getImages();

    // Create views for the images in the swapchain
    vk::ImageViewCreateInfo image_view_create_info = { .viewType = vk::ImageViewType::e2D, .format = swapchain_surface_format.format, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    for (auto image : swapchain_images) {
        image_view_create_info.image = image;
        swapchain_image_views.emplace_back(device, image_view_create_info);
    }

    // Create command pool
    vk::CommandPoolCreateInfo pool_info = { .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = queue_index };
    cmd_pool = vk::raii::CommandPool(device, pool_info);

    // Create command buffer
    cmd_bufs.clear();
    vk::CommandBufferAllocateInfo alloc_info = { .commandPool = *cmd_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    cmd_bufs = vk::raii::CommandBuffers(device, alloc_info);

    // Create sync objects
    present_sema = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    render_sema = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    draw_fence = vk::raii::Fence(device, vk::FenceCreateInfo());
    device.resetFences(*draw_fence);

    cmd_bufs[0].reset();
    advanceSwapchain();
    cmd_bufs[0].begin({});

    // Transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
    transitionImageLayoutForSwapchain(
        current_swapchain_image_idx,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},                                                        // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
    );

    vk::RenderingAttachmentInfo attachment_info = {
        .imageView = *swapchain_image_views[current_swapchain_image_idx],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .storeOp = vk::AttachmentStoreOp::eStore
    };
    vk::RenderingInfo render_info = {
        .renderArea = {.offset = { 0, 0 }, .extent = swapchain_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info
    };

    cmd_bufs[0].beginRendering(render_info);

    log("Using device %s\n", physical_device.getProperties().deviceName);
    log("Vulkan initialized successfully\n");
}

// Keep track of the pipelines we used this frame to cleanup state after flipping
std::vector<Pipeline*> curr_frame_pipelines;
// Keep track of index buffers needed for this frame and clear after flipping
std::vector<vk::raii::Buffer> idx_bufs;
std::vector<vk::raii::DeviceMemory> idx_buf_mems;

void VulkanRenderer::draw(const u64 cnt, const void* idx_buf_ptr) {
    const auto* vs_ptr = getVSPtr();
    const auto* ps_ptr = getPSPtr();
    log("Vertex Shader address : %p\n", vs_ptr);
    log("Pixel Shader address  : %p\n", ps_ptr);

    if (!vs_ptr || !ps_ptr) return;

    /*
    // Hack to skip embedded shader with no fetch shader
    u32* ptr = (u32*)vs_ptr;
    while (*ptr != 0x5362724F) {    // "OrbS"
        ptr++;
    }
    // Get the shader hash from the header
    u64 hash;
    ptr += 4;
    std::memcpy(&hash, ptr, sizeof(u64));
    if (hash == 0x75486d66862abd78) return;
    */

    //std::ofstream vs_dump;
    //vs_dump.open("vs_dump.bin", std::ios::binary);
    //vs_dump.write((char*)vs_ptr, 16_KB);

    // TODO: For now fetch the shader address is hardcoded to user register 0:1.
    // I think the proper way is to get the register from the SWAPPC instruction in the vertex shader...?
    const auto* fetch_shader_ptr = (u8*)((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_0] | ((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_1] << 32));
    log("Fetch Shader address : %p\n", fetch_shader_ptr);

    // Get pipeline
    auto& pipeline = Vulkan::PipelineCache::getPipeline(vs_ptr, ps_ptr, fetch_shader_ptr);
    curr_frame_pipelines.push_back(&pipeline);

    // Gather vertex data
    auto* vtx_bindings = pipeline.gatherVertices(cnt);

    // Upload buffers and get descriptor writes
    auto descriptor_writes = pipeline.uploadBuffersAndTextures();

    // Create index buffer (if needed)
    vk::raii::Buffer* vk_idx_buf_ptr = nullptr;
    if (idx_buf_ptr) {
        vk::DeviceSize idx_buf_size = cnt * sizeof(u16); // TODO: index type is stubbed

        auto& idx_buf = idx_bufs.emplace_back(nullptr);
        auto& idx_buf_mem = idx_buf_mems.emplace_back(nullptr);
        vk_idx_buf_ptr = &idx_buf;

        vk::raii::Buffer buf = vk::raii::Buffer(device, { .size = idx_buf_size, .usage = vk::BufferUsageFlagBits::eTransferSrc, .sharingMode = vk::SharingMode::eExclusive });
        auto mem_requirements = buf.getMemoryRequirements();
        vk::raii::DeviceMemory buf_mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
        buf.bindMemory(*buf_mem, 0);
        void* data = buf_mem.mapMemory(0, idx_buf_size);
        std::memcpy(data, idx_buf_ptr, idx_buf_size);
        buf_mem.unmapMemory();

        idx_buf = vk::raii::Buffer(device, { .size = idx_buf_size, .usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, .sharingMode = vk::SharingMode::eExclusive });
        mem_requirements = buf.getMemoryRequirements();
        idx_buf_mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) });
        idx_buf.bindMemory(*idx_buf_mem, 0);

        vk::raii::CommandBuffer tmp_cmd = beginCommands();
        tmp_cmd.copyBuffer(*buf, *idx_buf, vk::BufferCopy(0, 0, idx_buf_size));
        endCommands(tmp_cmd);
    }

    // Draw
    cmd_bufs[0].bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.getVkPipeline());

    if (descriptor_writes.size())
        cmd_bufs[0].pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, *pipeline.getVkPipelineLayout(), 0, descriptor_writes);

    cmd_bufs[0].setViewport(0, vk::Viewport(0.0f, (float)swapchain_extent.height, (float)swapchain_extent.width, -(float)swapchain_extent.height, 0.0f, 1.0f));
    cmd_bufs[0].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain_extent));

    for (int i = 0; i < vtx_bindings->size(); i++)
        cmd_bufs[0].bindVertexBuffers(i, *(*vtx_bindings)[i].buf, {0});
    
    if (idx_buf_ptr) {
        cmd_bufs[0].bindIndexBuffer(**vk_idx_buf_ptr, 0, vk::IndexType::eUint16);
        cmd_bufs[0].drawIndexed(cnt, 1, 0, 0, 0);
    }
    else {
        cmd_bufs[0].draw(cnt, 1, 0, 0);
    }
}

static bool fullscreen = false;

void VulkanRenderer::flip() {
    cmd_bufs[0].endRendering();
    // After rendering, transition the swapchain image to PRESENT_SRC
    transitionImageLayoutForSwapchain(
        current_swapchain_image_idx,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
        {},                                                        // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
    );
    cmd_bufs[0].end();

    vk::PipelineStageFlags wait_dest_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo  submit_info = { .waitSemaphoreCount = 1, .pWaitSemaphores = &*present_sema, .pWaitDstStageMask = &wait_dest_stage_mask, .commandBufferCount = 1, .pCommandBuffers = &*cmd_bufs[0], .signalSemaphoreCount = 1, .pSignalSemaphores = &*render_sema };
    queue.submit(submit_info, *draw_fence);

    auto recreate_swapchain = [&]() {
        device.waitIdle();
        cmd_bufs[0].reset();
        recreateSwapChain();
        advanceSwapchain();
        cmd_bufs[0].begin({});
        // Transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
        transitionImageLayoutForSwapchain(
            current_swapchain_image_idx,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},                                                        // srcAccessMask (no need to wait for previous operations)
            vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
            vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
        );
    };

    try {
        const vk::PresentInfoKHR present_info = { .waitSemaphoreCount = 1, .pWaitSemaphores = &*render_sema, .swapchainCount = 1, .pSwapchains = &*swapchain, .pImageIndices = &current_swapchain_image_idx };
        auto result = queue.presentKHR(present_info);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebuffer_resized) {
            framebuffer_resized = false;
            recreate_swapchain();
        }
        else if (result != vk::Result::eSuccess) {
            Helpers::panic("Vulkan: failed to present swapchain image!");
        }
    }
    catch (const vk::SystemError& e) {
        if (e.code().value() == (int)vk::Result::eErrorOutOfDateKHR) {
            framebuffer_resized = false;
            recreate_swapchain();
        }
        else {
            Helpers::panic("Vulkan: failed to present swapchain image!");
        }
    }

    // TODO: Move generic flip operations out of the vulkan specific code (i.e. SDL events, polling pads, FPS counter etc)

    // Handle SDL events
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT: {
            //exit(0);
            std::_Exit(0);
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button == SDL_BUTTON_LEFT && e.button.clicks == 2) {
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                SDL_ShowCursor(fullscreen ? SDL_DISABLE : SDL_ENABLE);
            }
            break;
        }

        case SDL_CONTROLLERDEVICEADDED: {
            if (!PS4::OS::Libs::ScePad::controller) {
                PS4::OS::Libs::ScePad::controller = SDL_GameControllerOpen(e.cdevice.which);
            }
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED: {
            if (PS4::OS::Libs::ScePad::controller && e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(PS4::OS::Libs::ScePad::controller))) {
                SDL_GameControllerClose(PS4::OS::Libs::ScePad::controller);
                PS4::OS::Libs::ScePad::controller = nullptr;
            }
            break;
        }
        }
    }

    // Update pads
    PS4::OS::Libs::ScePad::pollPads();

    // FPS counter
    frame_count++;
    const u64 curr_ticks = SDL_GetTicks64();
    const double curr_time = curr_ticks / 1000.0;
    if (curr_time - last_time > 1.0) {
        SDL_SetWindowTitle(window, std::format("ChonkyStation4 | {} | {} FPS", g_app.name, frame_count).c_str());
        last_time = curr_time;
        frame_count = 0;
    }

    // Wait for rendering to be done
    while (vk::Result::eTimeout == device.waitForFences(*draw_fence, vk::True, UINT64_MAX));
    device.resetFences(*draw_fence);

    // Cleanup
    for (auto& pipeline : curr_frame_pipelines)
        pipeline->clearBuffers();
    curr_frame_pipelines.clear();
    idx_bufs.clear();
    idx_buf_mems.clear();

    cmd_bufs[0].reset();
    advanceSwapchain();
    cmd_bufs[0].begin({});
    // Transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
    transitionImageLayoutForSwapchain(
        current_swapchain_image_idx,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},                                                        // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
    );
    vk::RenderingAttachmentInfo attachment_info = {
        .imageView = *swapchain_image_views[current_swapchain_image_idx],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .storeOp = vk::AttachmentStoreOp::eStore
    };
    vk::RenderingInfo render_info = {
        .renderArea = {.offset = { 0, 0 }, .extent = swapchain_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info
    };

    cmd_bufs[0].beginRendering(render_info);
}

}   // End namespace PS4::GCN::Vulkan