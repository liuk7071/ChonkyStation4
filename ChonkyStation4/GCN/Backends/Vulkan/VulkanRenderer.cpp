#pragma once

#define NOMINMAX
#include "VulkanRenderer.hpp"
#include <Logger.hpp>
#include <Loaders/App.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/PipelineCache.hpp>
#include <GCN/Backends/Vulkan/BufferCache.hpp>
#include <GCN/Backends/Vulkan/TextureCache.hpp>
#include <GCN/Backends/Vulkan/RenderTarget.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <SDL_vulkan.h>
#include <OS/Libraries/ScePad/ScePad.hpp>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


extern App g_app;

namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

constexpr bool enable_validation_layers = false;

const std::vector<char const*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
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
        [](const auto &format) { return format.format == vk::Format::eR8G8B8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
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
        .imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment,
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
    vk::ValidationFeaturesEXT validation_features;
    std::array validation_enabled = {
        //vk::ValidationFeatureEnableEXT::eGpuAssisted,
        vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
        vk::ValidationFeatureEnableEXT::eBestPractices
    };
    
    std::vector<char const*> required_layers;
    if (enable_validation_layers) {
        required_layers.assign(validation_layers.begin(), validation_layers.end());
        validation_features.setEnabledValidationFeatures(validation_enabled);
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
    required_exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    
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
        .pNext                   = enable_validation_layers ? &validation_features : nullptr,
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

        auto features                 = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT, vk::PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT, vk::PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT>();
        bool supports_all_required_features =    features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters
                                        && features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering
                                        && features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState
                                        && features.template get<vk::PhysicalDeviceFeatures2>().features.depthClamp
                                        && features.template get<vk::PhysicalDeviceFeatures2>().features.tessellationShader
                                        && features.template get<vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT>().dynamicRenderingUnusedAttachments
                                        && features.template get<vk::PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>().attachmentFeedbackLoopLayout
                                        && features.template get<vk::PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT>().attachmentFeedbackLoopDynamicState;

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
    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT, vk::PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT, vk::PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT> feature_chain = {
        { .features = { .depthClamp = true, .tessellationShader = true } },         // vk::PhysicalDeviceFeatures2
        { .shaderDrawParameters = true                                   },         // vk::PhysicalDeviceVulkan11Features
        { .synchronization2     = true, .dynamicRendering = true         },         // vk::PhysicalDeviceVulkan13Features
        { .extendedDynamicState = true                                   },         // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        { .dynamicRenderingUnusedAttachments = true                      },         // vk::PhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT
        { .attachmentFeedbackLoopLayout = true                           },         // vk::PhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT
        { .attachmentFeedbackLoopDynamicState = true                     }          // vk::PhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT
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
        .imageUsage       = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment,
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

    // Get host memory import alignment
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT host_props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT
    };
    VkPhysicalDeviceProperties2 props2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &host_props
    };
    vkGetPhysicalDeviceProperties2(*physical_device, &props2);
    host_memory_import_align = host_props.minImportedHostPointerAlignment;

    // Setup VulkanMemoryAllocator
    const VmaVulkanFunctions functions = {
        .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = &vkGetDeviceProcAddr,
    };

    const VmaAllocatorCreateInfo allocator_info = {
        .flags = 0,
        .physicalDevice = *physical_device,
        .device = *device,
        .pVulkanFunctions = &functions,
        .instance = *instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    vmaCreateAllocator(&allocator_info, &allocator);

    vk::BufferCreateInfo buf_create_info = {
        .size  = 4096,   // Doesn't matter, we just want the memory type bits
        .usage =   vk::BufferUsageFlagBits::eIndexBuffer   | vk::BufferUsageFlagBits::eVertexBuffer
                 | vk::BufferUsageFlagBits::eTransferSrc   | vk::BufferUsageFlagBits::eTransferDst
                 | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };
    vk::raii::Buffer dummy_buf = vk::raii::Buffer(device, buf_create_info);
    u32 mem_type_bits = dummy_buf.getMemoryRequirements().memoryTypeBits;

    // Create a memory pool. VMA will create allocations in block as specified below and manage memory by itself,
    // without having to allocate new Vulkan DeviceMemory every time.
    // This pool is used by the buffer cache (vertex buffers, index buffers, SSBOs)
    const VmaPoolCreateInfo vma_pool_info = {
        .flags = VmaPoolCreateFlagBits::VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
        .blockSize = 512_MB,
        .minBlockCount = 1,
        .maxBlockCount = 20,
        .memoryTypeIndex = findMemoryType(mem_type_bits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };
    vmaCreatePool(allocator, &vma_pool_info, &vma_pool);

    // Initialize the buffer cache
    Cache::init();

    log("Using device %s\n", physical_device.getProperties().deviceName);
    log("Vulkan initialized successfully\n");
}

// Keep track of the pipelines we used this frame to cleanup state after flipping
std::vector<Pipeline*> curr_frame_pipelines;
Pipeline*   last_draw_pipeline = nullptr;
ColorTarget last_color_rt[8] = {};
DepthTarget last_depth_rt = {};
RenderTarget::Attachment color_attachments[8] = {};
RenderTarget::Attachment depth_attachment = {};
float last_viewport_min_depth = 0.0f;
float last_viewport_max_depth = 0.0f;
bool last_depth_enable = false;
bool last_stencil_enable = false;
vk::Extent2D last_extent = vk::Extent2D { 0xffffffff, 0xffffffff };

void VulkanRenderer::draw(const u64 cnt, const void* idx_buf_ptr) {
    const auto* vs_ptr = getVSPtr();
    const auto* ps_ptr = getPSPtr();
    log("Vertex Shader address : %p\n", vs_ptr);
    log("Pixel Shader address  : %p\n", ps_ptr);

    // Temporary hack to skip embedded shaders with no fetch shader
    u32* ptr = (u32*)vs_ptr;
    while (*ptr != 0x5362724F) {    // "OrbS"
        ptr++;
    }
    // Get the shader hash from the header
    u64 hash;
    ptr += 4;
    std::memcpy(&hash, ptr, sizeof(u64));
    //printf("0x%llx <-- hash\n", hash);
    if (   hash == 0x75486d66862abd78   // Tomb Raider: Definitive Edition
        || hash == 0xf871bb9d4e8878f8   // Tomb Raider: Definitive Edition
        || hash == 0xd4f680821d9336a4   // Super Meat Boy
        || hash == 0x0000000042119848   // Super Meat Boy Forever
        || hash == 0x9b2da5cf47f8c29f   // libSceGnmDriver.sprx
       ) return;
    

    //std::ofstream vs_dump;
    //vs_dump.open("vs_dump.bin", std::ios::binary);
    //vs_dump.write((char*)vs_ptr, 16_KB);

    // TODO: For now fetch the shader address is hardcoded to user register 0:1.
    // I think the proper way is to get the register from the SWAPPC instruction in the vertex shader...?
    const auto* fetch_shader_ptr = (u8*)((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_0] | ((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_1] << 32));
    log("Fetch Shader address : %p\n", fetch_shader_ptr);

    if (!fetch_shader_ptr)
        return;

    // Get pipeline
    auto& pipeline = Vulkan::PipelineCache::getPipeline(vs_ptr, ps_ptr, fetch_shader_ptr, regs);
    curr_frame_pipelines.push_back(&pipeline);

    // Create index buffer (if needed)
    vk::Buffer vk_idx_buf = nullptr;
    size_t idx_buf_offs = 0;
    if (idx_buf_ptr) {
        vk::DeviceSize idx_buf_size = cnt * sizeof(u16); // TODO: index type is stubbed
        auto [idx_buf, offs, was_dirty] = Cache::getBuffer((void*)idx_buf_ptr, idx_buf_size);
        vk_idx_buf = idx_buf;
        idx_buf_offs = offs;
    }

    // ---- Setup render targets ----
    // We need to do this BEFORE uploading textures below, because that function relies on
    // getVulkanAttachmentForColorTarget to set a flag to detect feedback loops.
    bool needs_new_render_pass = false;
    std::vector<vk::RenderingAttachmentInfo> curr_attachments;
    curr_attachments.reserve(8);

    ColorTarget new_rt[8];
    getColorTargets(new_rt);

    const bool stencil_only = !pipeline.cfg.depth_control.depth_enable && pipeline.cfg.depth_control.stencil_enable;
    DepthTarget new_depth_rt;
    getDepthTarget(&new_depth_rt, stencil_only);

    // Check if any color or depth target was changed
    bool has_feedback_loop = false;
    vk::Extent2D extent = { 0xffffffff, 0xffffffff };
    if (((regs[Reg::mmCB_COLOR_CONTROL] >> 4) & 3) != 0) {
        for (int i = 0; i < 1; i++) {
            if (new_rt[i].enabled) {
                if (color_rt_dim[i].width < extent.width)   extent.width  = color_rt_dim[i].width;
                if (color_rt_dim[i].height < extent.height) extent.height = color_rt_dim[i].height;

                if (last_color_rt[i] != new_rt[i]) {
                    bool save = false;
                    color_attachments[i] = RenderTarget::getVulkanAttachmentForColorTarget(&new_rt[i], &save);
                    if (save)
                        last_color_rt[i] = new_rt[i];
                    else
                        last_color_rt[i] = {};
                    needs_new_render_pass = true;
                }

                curr_attachments.push_back(color_attachments[i].vk_attachment);
                if (color_attachments[i].has_feedback_loop) {
                    has_feedback_loop = true;
                    last_color_rt[i] = {};  // Next time we draw we need to check again if we are in a feedback loop, because it could keep the same color target but change input texture
                    if (color_attachments[i].tex->curr_layout != vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT) {
                        endRendering();
                        color_attachments[i].tex->transition(vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT);
                    }
                }
            }
            else {
                if (last_color_rt[i].enabled) {
                    needs_new_render_pass = true;
                    last_color_rt[i].enabled = false;
                }
            }
        }
    }
    else {
        needs_new_render_pass = true;
        std::memset(last_color_rt, 0, sizeof(ColorTarget) * 8);
    }

    if (pipeline.cfg.depth_control.depth_enable) {
        if (depth_rt_dim.width  < extent.width)  extent.width  = depth_rt_dim.width;
        if (depth_rt_dim.height < extent.height) extent.height = depth_rt_dim.height;

        if (   last_depth_rt != new_depth_rt
            || last_depth_enable != pipeline.cfg.depth_control.depth_enable
            || last_stencil_enable != pipeline.cfg.depth_control.stencil_enable
           ) {
            bool save = false;
            depth_attachment = RenderTarget::getVulkanAttachmentForDepthTarget(&new_depth_rt, pipeline.cfg.depth_control.stencil_enable, &save);

            if (save)
                last_depth_rt = new_depth_rt;
            else
                last_depth_rt = {};
            
            last_depth_enable = pipeline.cfg.depth_control.depth_enable;
            last_stencil_enable = pipeline.cfg.depth_control.stencil_enable;
            needs_new_render_pass = true;
        }
    }
    else {
        if (last_depth_rt.depth_base) {
            last_depth_rt = {};
            needs_new_render_pass = true;
        }
    }

    // Gather vertex data
    auto* vtx_bindings = pipeline.gatherVertices();

    // Upload buffers and get descriptor writes, as well as the push constants
    Pipeline::PushConstants* push_constants;
    auto descriptor_writes = pipeline.uploadBuffersAndTextures(&push_constants, color_attachments[0].tex, &has_feedback_loop);

    if (needs_new_render_pass || !is_recording_render_block) {
        endRendering(); // Has a check for if we were recording a render block or not

        if (extent == vk::Extent2D{ 0xffffffff, 0xffffffff })
            Helpers::panic("draw: no draw attachments\n");

        vk::RenderingInfo render_info = {
            .renderArea = { .offset = { 0, 0 }, .extent = extent },
            .layerCount = 1,
            .colorAttachmentCount = (u32)curr_attachments.size(),
            .pColorAttachments  = curr_attachments.size() ? curr_attachments.data() : nullptr,
            .pDepthAttachment   = pipeline.cfg.depth_control.depth_enable   ? &depth_attachment.vk_attachment : nullptr,
            .pStencilAttachment = pipeline.cfg.depth_control.stencil_enable ? &depth_attachment.vk_attachment : nullptr
        };

        beginRendering(render_info);
        cmd_bufs[0].setAttachmentFeedbackLoopEnableEXT(has_feedback_loop ? vk::ImageAspectFlagBits::eColor : vk::ImageAspectFlagBits::eNone);
    }

    // HACK: Skip feedback loops
    if (has_feedback_loop)
        return;

    // ---- Draw ----

    if (&pipeline != last_draw_pipeline) {
        cmd_bufs[0].bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.getVkPipeline());
        last_draw_pipeline = &pipeline;
    }

    // Viewport
    if (pipeline.min_viewport_depth != last_viewport_min_depth || pipeline.max_viewport_depth != last_viewport_max_depth || extent != last_extent) {
        cmd_bufs[0].setViewport(0, vk::Viewport(0.0f, (float)extent.height, (float)extent.width, -(float)extent.height, pipeline.min_viewport_depth, pipeline.max_viewport_depth));
        cmd_bufs[0].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));
        last_viewport_min_depth = pipeline.min_viewport_depth;
        last_viewport_max_depth = pipeline.max_viewport_depth;
        last_extent = extent;
    }

    if (descriptor_writes.size())
        cmd_bufs[0].pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, *pipeline.getVkPipelineLayout(), 0, descriptor_writes);
    
    // I couldn't figure out how to use the RAII version of this...
    vkCmdPushConstants(*cmd_bufs[0], *pipeline.getVkPipelineLayout(), static_cast<VkShaderStageFlagBits>(vk::ShaderStageFlagBits::eAllGraphics), 0, sizeof(Pipeline::PushConstants), push_constants);

    for (int i = 0; i < vtx_bindings->size(); i++)
        cmd_bufs[0].bindVertexBuffers(i, (*vtx_bindings)[i].buf, (*vtx_bindings)[i].offs_in_buf);
    
    if (idx_buf_ptr) {
        cmd_bufs[0].bindIndexBuffer(vk_idx_buf, idx_buf_offs, vk::IndexType::eUint16);
        cmd_bufs[0].drawIndexed(cnt, 1, 0, 0, 0);
    }
    else {
        cmd_bufs[0].draw(cnt, 1, 0, 0);
    }
}

static bool fullscreen = false;

void VulkanRenderer::flip(OS::Libs::SceVideoOut::SceVideoOutBuffer* buf) {
    endRendering();

    std::memset(last_color_rt, 0, sizeof(ColorTarget) * 8);
    last_depth_rt = {};

    // Get the SceVideoOut output buffer
    TSharp tsharp;
    tsharp.width    = buf->attrib.width - 1;
    tsharp.height   = buf->attrib.height - 1;
    tsharp.pitch    = buf->attrib.pitch_in_pixels - 1;
    tsharp.base_address = (uptr)buf->base >> 8;
    tsharp.data_format  = (u32)DataFormat::Format8_8_8_8; // TODO
    tsharp.num_format   = (u32)NumberFormat::Unorm;
    tsharp.dst_sel_x = DSEL_R;
    tsharp.dst_sel_y = DSEL_G;
    tsharp.dst_sel_z = DSEL_B;
    tsharp.dst_sel_w = DSEL_A;
    tsharp.tiling_index = 31;
    
    TrackedTexture* out_tex;
    getVulkanImageInfoForTSharp(&tsharp, &out_tex);

    out_tex->transition(vk::ImageLayout::eTransferSrcOptimal);
    transitionImageLayout(swapchain_images[current_swapchain_image_idx], vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal, &cmd_bufs[0]);

    vk::ImageBlit blit = {};
    blit.srcSubresource.aspectMask      = vk::ImageAspectFlagBits::eColor;
    blit.srcSubresource.mipLevel        = 0;
    blit.srcSubresource.baseArrayLayer  = 0;
    blit.srcSubresource.layerCount      = 1;
    blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
    blit.srcOffsets[1] = vk::Offset3D(buf->attrib.width, buf->attrib.height, 1);

    blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    blit.dstSubresource.mipLevel = 0;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;
    blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
    blit.dstOffsets[1] = vk::Offset3D(1920, 1080, 1);
    
    cmd_bufs[0].blitImage(out_tex->image, vk::ImageLayout::eTransferSrcOptimal, swapchain_images[current_swapchain_image_idx], vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
    transitionImageLayout(swapchain_images[current_swapchain_image_idx], vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR, &cmd_bufs[0]);
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
    last_draw_pipeline = nullptr;
    last_extent = vk::Extent2D{ 0xffffffff, 0xffffffff };
    Cache::clear();
    RenderTarget::reset();

    cmd_bufs[0].reset();
    advanceSwapchain();
    cmd_bufs[0].begin({});
}

}   // End namespace PS4::GCN::Vulkan