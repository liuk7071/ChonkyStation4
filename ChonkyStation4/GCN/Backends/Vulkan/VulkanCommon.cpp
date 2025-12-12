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

u32 findMemoryType(u32 type_filter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties mem_properties = physical_device.getMemoryProperties();

    for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Helpers::panic("findMemoryType: failed to find suitable memory type");
}

}   // End namespace PS4::GCN::Vulkan