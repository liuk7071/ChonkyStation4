#include "VulkanCommon.hpp"


namespace PS4::GCN::Vulkan {

vk::raii::CommandBuffer beginCommands() {
    vk::CommandBufferAllocateInfo alloc_info = { .commandPool = *cmd_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    vk::raii::CommandBuffer command_buffer = std::move(device.allocateCommandBuffers(alloc_info).front());

    vk::CommandBufferBeginInfo begin_info = { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    command_buffer.begin(begin_info);

    return command_buffer;
}

void endCommands(vk::raii::CommandBuffer& cmd_buffer) {
    cmd_buffer.end();

    vk::SubmitInfo submit_info = { .commandBufferCount = 1, .pCommandBuffers = &*cmd_buffer };
    queue.submit(submit_info, nullptr);
    queue.waitIdle();
}

void beginRendering(vk::RenderingInfo render_info) {
    is_recording_render_block = true;
    cmd_bufs[0].beginRendering(render_info);
}

void endRendering() {
    if (is_recording_render_block) {
        is_recording_render_block = false;
        cmd_bufs[0].endRendering();
    }
}

void transitionImageLayout(const vk::Image& image, const vk::Format fmt, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::raii::CommandBuffer* cmd_buf) {
    auto get_aspect = [](const vk::Format fmt) -> vk::ImageAspectFlagBits {
        switch (fmt) {
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
            return vk::ImageAspectFlagBits::eDepth;
        default:
            return vk::ImageAspectFlagBits::eColor;
        }
    };
    
    vk::ImageMemoryBarrier barrier = {
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .image = image,
        .subresourceRange = {
            get_aspect(fmt),
            0, 1, 
            0, 1
        }
    };
    
    auto get_access_and_stage_bits = [](vk::ImageLayout layout) -> std::pair<vk::Flags<vk::AccessFlagBits>, vk::Flags<vk::PipelineStageFlagBits>> {
        switch (layout) {
        case vk::ImageLayout::eUndefined:                           
            return { 
                {}, 
                vk::PipelineStageFlagBits::eTopOfPipe 
            };
        
        case vk::ImageLayout::eTransferSrcOptimal:    
            return {
                vk::AccessFlagBits::eTransferRead,
                vk::PipelineStageFlagBits::eTransfer
            };
        
        case vk::ImageLayout::eTransferDstOptimal:           
            return {
                vk::AccessFlagBits::eTransferWrite,
                vk::PipelineStageFlagBits::eTransfer
            };
        
        case vk::ImageLayout::eColorAttachmentOptimal:    
            return {
                vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, 
                vk::PipelineStageFlagBits::eColorAttachmentOutput
            };
        
        case vk::ImageLayout::eDepthAttachmentOptimal:  
            return { 
                vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite, 
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests 
            };
        
        case vk::ImageLayout::eShaderReadOnlyOptimal:         
            return {
                vk::AccessFlagBits::eShaderRead,
                vk::PipelineStageFlagBits::eAllGraphics
            };
        
        case vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT:   
            return {
                vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, 
                vk::PipelineStageFlagBits::eAllGraphics
            };
        
        case vk::ImageLayout::ePresentSrcKHR:     
            return {
                {},
                vk::PipelineStageFlagBits::eTopOfPipe
            };
        default:    Helpers::panic("transitionImageLayout: unhandled image layout %d\n", layout);
        }
    };

    if (!cmd_buf)
        cmd_buf = &cmd_bufs[0];

    auto [src_access_mask, src_stage] = get_access_and_stage_bits(old_layout);
    auto [dst_access_mask, dst_stage] = get_access_and_stage_bits(new_layout);

    if (   new_layout == vk::ImageLayout::eColorAttachmentOptimal
        //|| new_layout == vk::ImageLayout::eDepthAttachmentOptimal
        || new_layout == vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT
        ) {
        src_access_mask |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    }

    barrier.srcAccessMask = src_access_mask;
    barrier.dstAccessMask = dst_access_mask;
    cmd_buf->pipelineBarrier(src_stage, dst_stage, {}, {}, nullptr, barrier);
}

u32 findMemoryType(u32 type_filter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties mem_properties = physical_device.getMemoryProperties();

    for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Helpers::panic("findMemoryType: failed to find suitable memory type");
}

// Returns a Vulkan format alongside the size of 1 pixel in bytes
std::pair<vk::Format, size_t> getBufFormatAndSize(u32 dfmt, u32 nfmt) {
    switch ((DataFormat)dfmt) {

    case DataFormat::Format8: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm: return { vk::Format::eR8Unorm, sizeof(u8) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format16: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm:   return { vk::Format::eR16Unorm, sizeof(u16) };
        case NumberFormat::Float:   return { vk::Format::eR16Sfloat, sizeof(u16) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format32: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Float:   return { vk::Format::eR32Sfloat, sizeof(u32) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format16_16: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm:   return { vk::Format::eR16G16Unorm, sizeof(u16) * 2 };
        case NumberFormat::Snorm:   return { vk::Format::eR16G16Snorm, sizeof(u16) * 2 };
        case NumberFormat::Sscaled: return { vk::Format::eR16G16Sscaled, sizeof(u16) * 2 };
        case NumberFormat::Float:   return { vk::Format::eR16G16Sfloat, sizeof(u16) * 2 };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format2_10_10_10: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Snorm:   return { vk::Format::eA2B10G10R10SnormPack32, sizeof(u32) };    // TODO: Verify this

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format8_8_8_8: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm:   return { vk::Format::eR8G8B8A8Unorm, sizeof(u32) };
        case NumberFormat::Snorm:   return { vk::Format::eR8G8B8A8Snorm, sizeof(u32) };
        case NumberFormat::Uscaled: return { vk::Format::eR8G8B8A8Uscaled, sizeof(u32) };
        case NumberFormat::SnormNz: return { vk::Format::eR8G8B8A8Srgb, sizeof(u32) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format32_32: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Float: return { vk::Format::eR32G32Sfloat, sizeof(u32) * 2 };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format16_16_16_16: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Float: return { vk::Format::eR16G16B16A16Sfloat, sizeof(u16) * 4 };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format32_32_32: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Float: return { vk::Format::eR32G32B32Sfloat, sizeof(u32) * 3 };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format32_32_32_32: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Float: return { vk::Format::eR32G32B32A32Sfloat, sizeof(u32) * 4 };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format5_6_5: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm: return { vk::Format::eR5G6B5UnormPack16, sizeof(u16) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::FormatBc1: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm: return { vk::Format::eBc1RgbaUnormBlock, sizeof(u32) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::FormatBc3: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm: return { vk::Format::eBc3UnormBlock, sizeof(u32) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    // TODO: Fmask
    case DataFormat::FormatFmask8_4: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Uint: return { vk::Format::eR8Uint, sizeof(u8) };

        default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    default:    Helpers::panic("Unimplemented buffer/texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
    //default:    return { vk::Format::eR8G8B8A8Unorm, sizeof(u32) };
    }

    Helpers::panic("getBufFormatAndSize: unreachable\n");
}

}   // End namespace PS4::GCN::Vulkan