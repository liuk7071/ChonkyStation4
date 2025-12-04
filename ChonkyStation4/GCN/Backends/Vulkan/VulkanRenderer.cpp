#pragma once

#define NOMINMAX
#include "VulkanRenderer.hpp"
#include <Logger.hpp>
#include <Loaders/App.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <SDL_vulkan.h>
#include <OS/Libraries/ScePad/ScePad.hpp>


extern App g_app;

namespace PS4::GCN {

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
    assert(!available_formats.empty());
    const auto format_it = std::ranges::find_if(
        available_formats,
        [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
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
    return { std::clamp<u32>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
             std::clamp<u32>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height) };
}

vk::raii::ShaderModule VulkanRenderer::createShaderModule(const std::vector<u32>& code) {
    vk::ShaderModuleCreateInfo create_info = { .codeSize = code.size() * sizeof(u32), .pCode = code.data() };
    vk::raii::ShaderModule shader_module = { device, create_info };

    return shader_module;
}

u32 VulkanRenderer::findMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = physical_device.getMemoryProperties();

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanRenderer::recreateSwapChain() {

}

void VulkanRenderer::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout old_layout, vk::ImageLayout new_layout) {
    vk::raii::CommandBuffer tmp_cmd = beginCommands();
    vk::ImageMemoryBarrier barrier = { .oldLayout = old_layout, .newLayout = new_layout, .image = *image, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    vk::PipelineStageFlags source_stage;
    vk::PipelineStageFlags destination_stage;

    if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
        destination_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        source_stage = vk::PipelineStageFlagBits::eTransfer;
        destination_stage = vk::PipelineStageFlagBits::eAllGraphics;
    }
    else {
        Helpers::panic("Unsupported layout transition\n");
    }
    tmp_cmd.pipelineBarrier(source_stage, destination_stage, {}, {}, nullptr, barrier);
    endCommands(tmp_cmd);
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
    auto [result, image_idx] = swapchain.acquireNextImage(UINT64_MAX, *present_sema, nullptr);
    current_swapchain_image_idx = image_idx;

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        Helpers::panic("Vulkan: failed to acquire swapchain image!");
    }

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
}

vk::raii::CommandBuffer VulkanRenderer::beginCommands() {
    vk::CommandBufferAllocateInfo alloc_info = { .commandPool = *cmd_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    vk::raii::CommandBuffer command_buffer = std::move(device.allocateCommandBuffers(alloc_info).front());

    vk::CommandBufferBeginInfo begin_info = { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    command_buffer.begin(begin_info);

    return command_buffer;
}

void VulkanRenderer::endCommands(vk::raii::CommandBuffer& cmd_buffer) {
    cmd_buffer.end();

    vk::SubmitInfo submit_info = { .commandBufferCount = 1, .pCommandBuffers = &*cmd_buffer };
    queue.submit(submit_info, nullptr);
    queue.waitIdle();
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
        if (std::ranges::none_of(layer_properties,
                                    [required_layer](auto const& layerProperty)
                                    { return strcmp(layerProperty.layerName, required_layer) == 0; })) {
            throw std::runtime_error("Required layer not supported: " + std::string(required_layer));
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
        if (std::ranges::none_of(extension_properties,
                                    [required_exts](auto const& extensionProperty)
                                    { return strcmp(extensionProperty.extensionName, required_exts) == 0; })) {
            throw std::runtime_error("Required extension not supported: " + std::string(required_exts));
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
    else throw std::runtime_error("Vulkan: failed to find a suitable GPU!");

    // Get the first index into queue_family_properties which supports both graphics and present
    std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device.getQueueFamilyProperties();
    for (u32 qfp_idx = 0; qfp_idx < queue_family_properties.size(); qfp_idx++) {
        if ((queue_family_properties[qfp_idx].queueFlags & vk::QueueFlagBits::eGraphics) && physical_device.getSurfaceSupportKHR(qfp_idx, *surface)) {
            // Found a queue family that supports both graphics and present
            queue_index = qfp_idx;
            break;
        }
    }
    if (queue_index == ~0) throw std::runtime_error("Could not find a queue family for graphics and presentation");

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
    vk::DeviceCreateInfo      device_create_info      = { .pNext                   = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
                                                        .queueCreateInfoCount    = 1,
                                                        .pQueueCreateInfos       = &device_queue_crerate_info,
                                                        .enabledExtensionCount   = static_cast<u32>(required_device_exts.size()),
                                                        .ppEnabledExtensionNames = required_device_exts.data() };

    device = vk::raii::Device(physical_device, device_create_info);
    queue  = vk::raii::Queue(device, queue_index, 0);

    // Create swapchain
    auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface);
    swapchain_extent          = chooseSwapExtent(window, surface_capabilities);
    swapchain_surface_format   = chooseSwapSurfaceFormat(physical_device.getSurfaceFormatsKHR(*surface));
    vk::SwapchainCreateInfoKHR swapchain_create_info = { .surface          = *surface,
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
                                                       .clipped          = true };

    swapchain        = vk::raii::SwapchainKHR(device, swapchain_create_info);
    swapchain_images = swapchain.getImages();

    // Create views for the images in the swapchain
    assert(swapchain_image_views.empty());
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
    cmd_bufs[0].begin({});
    advanceSwapchain();

    log("Using device %s\n", physical_device.getProperties().deviceName);
    log("Vulkan initialized successfully\n");
}

vk::Format VulkanRenderer::getVtxBufferFormat(u32 n_elements, u32 type) {
    // TODO: for now type is ignored
    switch (n_elements) {
    case 2:     return vk::Format::eR32G32Sfloat;
    case 3:     return vk::Format::eR32G32B32Sfloat;
    case 4:     return vk::Format::eR32G32B32A32Sfloat;
    default:    Helpers::panic("Vulkan: getVtxBuffeFormat unhandled n_elements=%d\n", n_elements);
    }
}

std::pair<vk::Format, size_t> VulkanRenderer::getTexFormatAndSize(u32 dfmt, u32 nfmt) {
    switch ((DataFormat)dfmt) {

    case DataFormat::Format5_6_5: {
        switch ((NumberFormat)nfmt) {
        
        case NumberFormat::Unorm: return { vk::Format::eR5G6B5UnormPack16, sizeof(u16)};

        default:    Helpers::panic("Unimplemented texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    default:    Helpers::panic("Unimplemented texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
    }

    Helpers::panic("getTexFormatAndSize: unreachable\n");
}

void VulkanRenderer::draw(const u64 cnt, const void* idx_buf_ptr) {
    const auto* vs_ptr = getVSPtr();
    const auto* ps_ptr = getPSPtr();
    log("Vertex Shader address : %p\n", vs_ptr);
    log("Pixel Shader address  : %p\n", ps_ptr);

    if (!vs_ptr || !ps_ptr) return;

    // TODO: For now fetch shader address is hardcoded to user register 0:1.
    // I think the proper way is to get the register from the SWAPPC instruction in the vertex shader...?
    const auto* fetch_shader_ptr = (u8*)((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_0] | ((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_1] << 32));
    log("Fetch Shader address : %p\n", fetch_shader_ptr);

    // TODO: Pipeline cache...

    FetchShader fetch_shader = FetchShader(fetch_shader_ptr);

    // Compile shaders
    Shader::ShaderData vert_shader, pixel_shader;
    Shader::decompileShader((u32*)vs_ptr, Shader::ShaderStage::Vertex, vert_shader, &fetch_shader);
    Shader::decompileShader((u32*)ps_ptr, Shader::ShaderStage::Fragment, pixel_shader);

    // Iterate over fetch shader bindings and convert them to vulkan binding/attribute descriptions, and create the vertex buffers
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attribs;
    u32 n_binding = 0;
    for (auto& shader_binding : fetch_shader.bindings) {
        // Get pointer to the V#
        VSharp* vsharp = shader_binding.vsharp_loc.asPtr();

        auto& binding = bindings.emplace_back();
        auto& attrib  = attribs.emplace_back();
        auto& buf     = vtx_bufs.emplace_back(nullptr);
        auto& mem     = vtx_bufs_mem.emplace_back(nullptr);
        binding = { n_binding, (u32)vsharp->stride, vk::VertexInputRate::eVertex };
        attrib = { shader_binding.dest_vgpr, n_binding++, getVtxBufferFormat(shader_binding.n_elements, 0 /* TODO: type */), 0 };

        // Setup vertex buffer and copy data
        const auto buf_size = vsharp->stride * cnt;
        buf = vk::raii::Buffer(device, { .size = buf_size, .usage = vk::BufferUsageFlagBits::eVertexBuffer, .sharingMode = vk::SharingMode::eExclusive });
        auto mem_requirements = buf.getMemoryRequirements();
        mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
        buf.bindMemory(*mem, 0);
        void* vtx_buf_data = mem.mapMemory(0, buf_size);
        void* guest_vtx_buf_data = (void*)(vsharp->base + shader_binding.voffs);
        std::memcpy(vtx_buf_data, guest_vtx_buf_data, buf_size);
        mem.unmapMemory();

        log("Created attribute binding for location %d\n", shader_binding.dest_vgpr);
    }

    // Create and upload buffers required by each shader
    std::vector<vk::DescriptorPoolSize> pool_size;
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
    std::vector<vk::DescriptorBufferInfo> buffer_info;
    std::vector<vk::DescriptorImageInfo> image_info;
    std::vector<vk::WriteDescriptorSet> descriptor_writes;

    std::vector<vk::DescriptorSetLayoutBinding> buf_bindings;
    auto create_buffers = [&](Shader::ShaderData data) {
        for (auto& buf_info : data.buffers) {
            switch (buf_info.desc_info.type) {
            case Shader::DescriptorType::Vsharp: {
                // Get pointer to the V#
                VSharp* vsharp = buf_info.desc_info.asPtr<VSharp>();

                // Upload as SSBO
                auto& buf = bufs.emplace_back(nullptr);
                auto& mem = bufs_mem.emplace_back(nullptr);
                auto& binding = buf_bindings.emplace_back();
                binding = { (u32)buf_info.binding, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll, nullptr };

                const auto buf_size = (vsharp->stride == 0 ? 1 : vsharp->stride) * vsharp->num_records;
                buf = vk::raii::Buffer(device, { .size = buf_size, .usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive });
                auto mem_requirements = buf.getMemoryRequirements();
                mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
                buf.bindMemory(*mem, 0);
                void* buf_data = mem.mapMemory(0, buf_size);
                void* guest_buf_data = (void*)(vsharp->base << 8);
                std::memcpy(buf_data, guest_buf_data, buf_size);
                mem.unmapMemory();

                pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1));
                layout_bindings.push_back(vk::DescriptorSetLayoutBinding(buf_info.binding, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr));
                buffer_info.push_back({
                    .buffer = *buf,
                    .offset = 0,
                    .range = buf_size,
                });
                descriptor_writes.push_back(vk::WriteDescriptorSet {
                    .dstSet = nullptr,  // Will be updated below after we actually allocate the descriptor set
                    .dstBinding = (u32)buf_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &buffer_info.back()
                });
                break;
            }

            case Shader::DescriptorType::Tsharp: {
                TSharp* tsharp = buf_info.desc_info.asPtr<TSharp>();
                const u32 width = tsharp->width + 1;
                const u32 height = tsharp->height + 1;
                const u32 pitch = tsharp->pitch + 1;

                const auto [vk_fmt, pixel_size] = getTexFormatAndSize(tsharp->data_format, tsharp->num_format);
                const size_t img_size = pitch * height * pixel_size;
                log("texture size: width=%lld, height=%lld\n", (u32)tsharp->width + 1, (u32)tsharp->height + 1);
                log("texture ptr: %p\n", (void*)tsharp->base_address);
                log("texture dfmt: %d\n", (u32)tsharp->data_format);
                log("texture nfmt: %d\n", (u32)tsharp->num_format);
                log("texture pitch: %d\n", (u32)tsharp->pitch + 1);

                vk::raii::Buffer tex_buf = vk::raii::Buffer(device, { .size = img_size, .usage = vk::BufferUsageFlagBits::eTransferSrc, .sharingMode = vk::SharingMode::eExclusive });
                auto mem_requirements = tex_buf.getMemoryRequirements();
                vk::raii::DeviceMemory tex_mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
                tex_buf.bindMemory(*tex_mem, 0);
                void* data = tex_mem.mapMemory(0, img_size);
                std::memcpy(data, (void*)(tsharp->base_address << 8), img_size);
                tex_mem.unmapMemory();

                // Create image
                auto& img = textures.emplace_back(nullptr);
                auto& mem = texture_mem.emplace_back(nullptr);
                vk::ImageCreateInfo img_info = {
                    .imageType = vk::ImageType::e2D,
                    .format = vk_fmt,
                    .extent = { width, height, 1 },
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = vk::SampleCountFlagBits::e1,
                    .tiling = vk::ImageTiling::eLinear,
                    .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                    .sharingMode = vk::SharingMode::eExclusive
                };
                img = vk::raii::Image(device, img_info);
                mem_requirements = img.getMemoryRequirements();
                vk::MemoryAllocateInfo alloc_info = { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
                mem = vk::raii::DeviceMemory(device, alloc_info);
                img.bindMemory(*mem, 0);
                transitionImageLayout(img, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

                // Copy buffer to image
                vk::raii::CommandBuffer tmp_cmd = beginCommands();
                vk::BufferImageCopy region = { .bufferOffset = 0, .bufferRowLength = pitch, .bufferImageHeight = 0, .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, .imageOffset = { 0, 0, 0 }, .imageExtent = { width, height, 1 } };
                tmp_cmd.copyBufferToImage(*tex_buf, *img, vk::ImageLayout::eTransferDstOptimal, { region });
                endCommands(tmp_cmd);

                transitionImageLayout(img, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

                // Create image view
                auto& img_view = texture_views.emplace_back(nullptr);
                vk::ImageViewCreateInfo view_info = { .image = *img, .viewType = vk::ImageViewType::e2D, .format = vk_fmt, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
                img_view = vk::raii::ImageView(device, view_info);

                // Create image sampler
                auto& sampler = texture_samplers.emplace_back(nullptr);
                vk::PhysicalDeviceProperties properties = physical_device.getProperties();
                vk::SamplerCreateInfo sampler_info = {
                    .magFilter = vk::Filter::eNearest,
                    .minFilter = vk::Filter::eNearest,
                    .mipmapMode = vk::SamplerMipmapMode::eNearest,
                    .addressModeU = vk::SamplerAddressMode::eRepeat,
                    .addressModeV = vk::SamplerAddressMode::eRepeat,
                    .addressModeW = vk::SamplerAddressMode::eRepeat,
                    .mipLodBias = 0.0f,
                    .anisotropyEnable = vk::False,
                    .maxAnisotropy = 1.0f,
                    .compareEnable = vk::False,
                    .compareOp = vk::CompareOp::eAlways,
                };
                sampler = vk::raii::Sampler(device, sampler_info);

                pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1));
                layout_bindings.push_back(vk::DescriptorSetLayoutBinding(buf_info.binding, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr));
                image_info.push_back({
                    .sampler = *sampler,
                    .imageView = *img_view,
                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                });
                descriptor_writes.push_back(vk::WriteDescriptorSet{
                    .dstSet = nullptr,  // Will be updated below after we actually allocate the descriptor set
                    .dstBinding = (u32)buf_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &image_info.back()
                });
                break;
            }
            }
        }
    };

    create_buffers(vert_shader);
    create_buffers(pixel_shader);

    // Create descriptor pool
    vk::DescriptorPoolCreateInfo pool_info = {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1,
        .poolSizeCount = static_cast<uint32_t>(pool_size.size()),
        .pPoolSizes = pool_size.data()
    };
    descriptor_pool = vk::raii::DescriptorPool(device, pool_info);

    // Create descriptor set
    vk::DescriptorSetLayoutCreateInfo layout_info = { .bindingCount = (u32)layout_bindings.size(), .pBindings = layout_bindings.data() };
    descriptor_set_layout = vk::raii::DescriptorSetLayout(device, layout_info);

    vk::DescriptorSetAllocateInfo alloc_info = {
        .descriptorPool = *descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &*descriptor_set_layout
    };
    descriptor_sets.clear();
    descriptor_sets = device.allocateDescriptorSets(alloc_info);

    // Update descriptor writes to point to the newly allocated descriptor set
    for (auto& write : descriptor_writes) write.dstSet = *descriptor_sets[0];

    device.updateDescriptorSets(descriptor_writes, {});

    // Create index buffer (if needed)
    if (idx_buf_ptr) {
        vk::DeviceSize idx_buf_size = cnt * sizeof(u16); // TODO: index type is stubbed

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

    // Setup graphics pipeline
    vk::raii::ShaderModule vert_shader_module = createShaderModule(PS4::GCN::compileGLSL(vert_shader.source, EShLangVertex));
    vk::raii::ShaderModule frag_shader_module = createShaderModule(PS4::GCN::compileGLSL(pixel_shader.source, EShLangFragment));
    vk::PipelineShaderStageCreateInfo vert_stage_info = { .stage = vk::ShaderStageFlagBits::eVertex, .module = *vert_shader_module, .pName = "main" };
    vk::PipelineShaderStageCreateInfo frag_stage_info = { .stage = vk::ShaderStageFlagBits::eFragment, .module = *frag_shader_module, .pName = "main" };
    vk::PipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

    vk::PipelineVertexInputStateCreateInfo   vertex_input_info = { .vertexBindingDescriptionCount = (u32)bindings.size(), .pVertexBindingDescriptions = bindings.data(), .vertexAttributeDescriptionCount = (u32)attribs.size(), .pVertexAttributeDescriptions = attribs.data() };
    vk::PipelineInputAssemblyStateCreateInfo input_assembly = { .topology = vk::PrimitiveTopology::eTriangleList };
    vk::PipelineViewportStateCreateInfo      viewport_state = { .viewportCount = 1, .scissorCount = 1 };

    vk::PipelineRasterizationStateCreateInfo rasterizer = { .depthClampEnable = vk::False, .rasterizerDiscardEnable = vk::False, .polygonMode = vk::PolygonMode::eFill, .cullMode = vk::CullModeFlagBits::eNone, .frontFace = vk::FrontFace::eClockwise, .depthBiasEnable = vk::False, .depthBiasSlopeFactor = 1.0f, .lineWidth = 1.0f };
    vk::PipelineMultisampleStateCreateInfo multisampling = { .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };
    vk::PipelineColorBlendAttachmentState color_blend_attachment = { .blendEnable = vk::False, .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };
    vk::PipelineColorBlendStateCreateInfo color_blending = { .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &color_blend_attachment };

    std::vector dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic_state = { .dynamicStateCount = (u32)dynamic_states.size(), .pDynamicStates = dynamic_states.data() };

    vk::PipelineLayoutCreateInfo pipeline_layout_info = { .setLayoutCount = 1, .pSetLayouts = &*descriptor_set_layout, .pushConstantRangeCount = 0 };
    pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_info);

    vk::GraphicsPipelineCreateInfo gpci = {};
    gpci.stageCount = 2;
    gpci.pStages = shader_stages;
    gpci.pVertexInputState = &vertex_input_info;
    gpci.pInputAssemblyState = &input_assembly;
    gpci.pViewportState = &viewport_state;
    gpci.pRasterizationState = &rasterizer;
    gpci.pMultisampleState = &multisampling;
    gpci.pColorBlendState = &color_blending;
    gpci.pDynamicState = &dynamic_state;
    gpci.layout = *pipeline_layout;
    gpci.renderPass = VK_NULL_HANDLE;
    vk::PipelineRenderingCreateInfo rendering_info = {};
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &swapchain_surface_format.format;

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info_chain(gpci, rendering_info);
    graphics_pipeline = vk::raii::Pipeline(device, nullptr, pipeline_create_info_chain.get());

    // Draw
    vk::RenderingAttachmentInfo attachment_info = {
        .imageView = *swapchain_image_views[current_swapchain_image_idx],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .storeOp = vk::AttachmentStoreOp::eStore };
    vk::RenderingInfo render_info = {
        .renderArea = { .offset = { 0, 0 }, .extent = swapchain_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachment_info };

    cmd_bufs[0].beginRendering(render_info);
    cmd_bufs[0].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphics_pipeline);
    cmd_bufs[0].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout, 0, *descriptor_sets[0], nullptr);
    cmd_bufs[0].setViewport(0, vk::Viewport(0.0f, (float)swapchain_extent.height, (float)swapchain_extent.width, -(float)swapchain_extent.height, 0.0f, 1.0f));
    cmd_bufs[0].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchain_extent));
    for (int i = 0; i < vtx_bufs.size(); i++)
        cmd_bufs[0].bindVertexBuffers(i, *vtx_bufs[i], { 0 });
    if (idx_buf_ptr) {
        cmd_bufs[0].bindIndexBuffer(*idx_buf, 0, vk::IndexType::eUint16);
        cmd_bufs[0].drawIndexed(cnt, 1, 0, 0, 0);
    }
    else {
        cmd_bufs[0].draw(cnt, 1, 0, 0);
    }
    cmd_bufs[0].endRendering();
}

void VulkanRenderer::flip() {
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

    try {
        const vk::PresentInfoKHR present_info = { .waitSemaphoreCount = 1, .pWaitSemaphores = &*render_sema, .swapchainCount = 1, .pSwapchains = &*swapchain, .pImageIndices = &current_swapchain_image_idx };
        auto result = queue.presentKHR(present_info);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebuffer_resized) {
            framebuffer_resized = false;
            recreateSwapChain();
        }
        else if (result != vk::Result::eSuccess) {
            Helpers::panic("Vulkan: failed to present swapchain image!");
        }
    }
    catch (const vk::SystemError& e) {
        if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR)) {
            recreateSwapChain();
            return;
        }
        else throw;
    }

    // TODO: Move generic flip operations out of the vulkan specific code (i.e. SDL events, polling pads, FPS counter etc)

    // Handle SDL events
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT: {
            exit(0);
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

    cmd_bufs[0].reset();
    vtx_bufs.clear();
    vtx_bufs_mem.clear();
    bufs.clear();
    bufs_mem.clear();
    textures.clear();
    texture_mem.clear();
    texture_views.clear();
    graphics_pipeline.~Pipeline();
    cmd_bufs[0].begin({});
    advanceSwapchain();
}

}   // End namespace PS4::GCN