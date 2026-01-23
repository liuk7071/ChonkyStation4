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

void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout old_layout, vk::ImageLayout new_layout) {
    vk::raii::CommandBuffer tmp_cmd = beginCommands();
    vk::ImageMemoryBarrier barrier = { .oldLayout = old_layout, .newLayout = new_layout, .image = *image, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    vk::PipelineStageFlags source_stage;
    vk::PipelineStageFlags destination_stage;

    if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        source_stage      = vk::PipelineStageFlagBits::eTopOfPipe;
        destination_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        source_stage      = vk::PipelineStageFlagBits::eTransfer;
        destination_stage = vk::PipelineStageFlagBits::eAllGraphics;
    }
    else if (old_layout == vk::ImageLayout::eShaderReadOnlyOptimal && new_layout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        source_stage      = vk::PipelineStageFlagBits::eAllGraphics;
        destination_stage = vk::PipelineStageFlagBits::eTransfer;
    }
    else {
        Helpers::panic("Unsupported layout transition\n");
    }
    tmp_cmd.pipelineBarrier(source_stage, destination_stage, {}, {}, nullptr, barrier);
    endCommands(tmp_cmd);
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