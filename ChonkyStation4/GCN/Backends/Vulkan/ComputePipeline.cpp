#include "ComputePipeline.hpp"
#include <Logger.hpp>
#include <Profiler.hpp>
#include <Configuration.hpp>
#include <GCN/HostTessShaders.hpp>
#include <GCN/Backends/Vulkan/BufferCache.hpp>
#include <GCN/Backends/Vulkan/TextureCache.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Detiler/gpuaddr.h>
#include <GCN/Detiler/gnm/texture.h>


namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

ComputePipeline::ComputePipeline(ShaderCache::CachedShader* compute_shader) : compute_shader(compute_shader) {
    //Profiler::Scope profiler("Compute Pipeline creation");

    // Create compute shader stage
    vk::PipelineShaderStageCreateInfo stage_info = { .stage = vk::ShaderStageFlagBits::eCompute, .module = compute_shader->vk_shader, .pName = "main" };

    // Create layout bindings
    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
    auto create_layout_bindings = [&](Shader::ShaderData data) {
        for (auto& buf_info : data.buffers) {
            switch (buf_info.desc_info.type) {
            case Shader::DescriptorType::Vsharp: {
                layout_bindings.push_back(vk::DescriptorSetLayoutBinding(buf_info.binding, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute, nullptr));
                break;
            }

            case Shader::DescriptorType::Tsharp: {
                layout_bindings.push_back(vk::DescriptorSetLayoutBinding(buf_info.binding, !buf_info.is_image_store ? vk::DescriptorType::eCombinedImageSampler : vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute, nullptr));
                break;
            }
            }
        }
    };

    create_layout_bindings(compute_shader->data);

    // Create the descriptor set layout
    vk::DescriptorSetLayoutCreateInfo layout_info = {
        .flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR,
        .bindingCount = (u32)layout_bindings.size(),
        .pBindings = layout_bindings.data()
    };
    descriptor_set_layout = vk::raii::DescriptorSetLayout(device, layout_info);

    // Create the pipeline layout
    vk::PushConstantRange push_constant_range = {
        vk::ShaderStageFlagBits::eCompute,
        0,
        sizeof(PushConstants)
    };
    vk::PipelineLayoutCreateInfo pipeline_layout_info = { .setLayoutCount = 1, .pSetLayouts = &*descriptor_set_layout, .pushConstantRangeCount = 1, .pPushConstantRanges = &push_constant_range };
    pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_info);

    // Create compute pipeline
    vk::ComputePipelineCreateInfo cpci = {
        .stage = stage_info,
        .layout = *pipeline_layout
    };
    compute_pipeline = vk::raii::Pipeline(device, nullptr, cpci);
}

std::vector<vk::WriteDescriptorSet> ComputePipeline::uploadBuffersAndTextures(PushConstants** push_constants_ptr, TrackedTexture* rt, bool* has_feedback_loop) {
    // Create and upload buffers required by each shader
    std::vector<vk::WriteDescriptorSet> descriptor_writes;
    descriptor_writes.reserve(32);
    std::memset(&push_constants, 0, sizeof(PushConstants));

    *has_feedback_loop = false;

    auto create_buffers = [&](Shader::ShaderData& data) {
        for (auto& buf_info : data.buffers) {
            switch (buf_info.desc_info.type) {
            case Shader::DescriptorType::Vsharp: {
                auto null_descriptor = [&]() {
                    buffer_info[frame_idx].push_back({
                        .buffer = VK_NULL_HANDLE,
                        .offset = 0,
                        .range = VK_WHOLE_SIZE,
                    });
                    
                    descriptor_writes.push_back(vk::WriteDescriptorSet {
                        .dstSet = nullptr,  // Not used for push descriptors
                        .dstBinding = (u32)buf_info.binding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eStorageBuffer,
                        .pBufferInfo = &buffer_info[frame_idx].back()
                    });
                };

                // Get pointer to the V#
                VSharp* vsharp = buf_info.desc_info.asPtr<VSharp>();
                // Skip bad shaders until I fix them...
                if ((u64)vsharp < 0x1000) {
                    null_descriptor();
                    continue;
                }

                // Upload as SSBO
                const auto buf_size = Helpers::alignUp<size_t>((vsharp->stride == 0 ? 1 : vsharp->stride) * vsharp->num_records, 16);
                void* guest_buf_data = (void*)vsharp->base;

                if ((u64)guest_buf_data < 0x10000 || buf_size == 0) {
                    null_descriptor();
                    continue;
                }

                if (Configuration::clamp_gpu_buffers) {
                    if (IsBadReadPtr((const void*)vsharp->base, buf_size)) {
                        null_descriptor();
                        continue;
                        //buf_size = clamp_size((uptr)vsharp->base, buf_size);
                        //Helpers::panic("Invalid vsharp->base %p for shader %llx\n", guest_buf_data, data.hash);
                    }
                }

                auto [cached_buf, offs, was_dirty] = Cache::getBuffer(guest_buf_data, buf_size);

                buffer_info[frame_idx].push_back({
                    .buffer = cached_buf,
                    .offset = offs,
                    .range = buf_size,
                });
                descriptor_writes.push_back(vk::WriteDescriptorSet {
                    .dstSet = nullptr,  // Not used for push descriptors
                    .dstBinding = (u32)buf_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .pBufferInfo = &buffer_info[frame_idx].back()
                });

                // Write push constants for this buffer
                if (buf_info.binding < 48) {
                    push_constants.stride[buf_info.binding] = vsharp->stride;
                    //push_constants.fmt[buf_info.binding]    = vsharp->dfmt | (vsharp->nfmt << 8);
                }
                else printf("TODO: buf_info.binding >= 48\n");
                break;
            }

            case Shader::DescriptorType::Tsharp: {
                auto null_descriptor = [&]() {
                    descriptor_writes.push_back(vk::WriteDescriptorSet {
                        .dstSet = nullptr,  // Not used for push descriptors
                        .dstBinding = (u32)buf_info.binding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                        .pImageInfo = &dummy_descriptor_image_info
                    });
                };

                TSharp* tsharp = buf_info.desc_info.asPtr<TSharp>();
                if (tsharp->data_format == 0) {
                    null_descriptor();
                    continue;
                }

                //if (IsBadReadPtr((const void*)(tsharp->base_address << 8), 1))
                //    Helpers::panic("Invalid tsharp->base_address %p for shader %llx\n", tsharp->base_address << 8, data.hash);

                TrackedTexture* tex;
                Vulkan::getVulkanImageInfoForTSharp(tsharp, &tex, true);
                tex->was_bound = true;

                if (tex == rt) {
                    *has_feedback_loop = true;
                }

                if (tex->curr_layout != vk::ImageLayout::eGeneral) {
                    endRendering();
                    tex->transition(vk::ImageLayout::eGeneral);
                }
                //if (image_info == nullptr) break;

                descriptor_writes.push_back(vk::WriteDescriptorSet{
                    .dstSet = nullptr,  // Not used for push descriptors
                    .dstBinding = (u32)buf_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = !buf_info.is_image_store ? vk::DescriptorType::eCombinedImageSampler : vk::DescriptorType::eStorageImage,
                    .pImageInfo = &tex->image_info_general
                });
                break;
            }
            }
        }
    };

    create_buffers(compute_shader->data);

    *push_constants_ptr = &push_constants;
    return descriptor_writes;
}

void ComputePipeline::clearBuffers() {
    buffer_info[frame_idx].clear();
}

}   // End namespace PS4::GCN::Vulkan