#include "Pipeline.hpp"
#include <Logger.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/TextureCache.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>
#include <GCN/Detiler/gpuaddr.h>
#include <GCN/Detiler/gnm/texture.h>


namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

Pipeline::Pipeline(Shader::ShaderData vert_shader, Shader::ShaderData pixel_shader, FetchShader fetch_shader) : vert_shader(vert_shader), pixel_shader(pixel_shader), fetch_shader(fetch_shader) {
    // Iterate over fetch shader bindings and convert them to vulkan binding/attribute descriptions, and create the vertex buffers
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attribs;
    u32 n_binding = 0;
    for (auto& shader_binding : fetch_shader.bindings) {
        // Get pointer to the V#
        VSharp* vsharp = shader_binding.vsharp_loc.asPtr();
        auto& binding = bindings.emplace_back();
        auto& attrib = attribs.emplace_back();
        
        // TODO: Given that the vsharp could theoretically change between draws, add an assert (probably on each gatherVertices) to check if the stride of
        // the current vsharp is different from the one which was set here.
        binding = { n_binding, (u32)vsharp->stride, vk::VertexInputRate::eVertex };
        attrib = { shader_binding.idx, n_binding++, getBufFormatAndSize(vsharp->dfmt, vsharp->nfmt).first, 0 };

        auto& vtx_binding = vtx_binding_layout.emplace_back();
        vtx_binding = shader_binding;
        log("Created attribute binding for location %d\n", shader_binding.idx);
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
    
    //vk::PipelineColorBlendAttachmentState color_blend_attachment = { .blendEnable = vk::False, .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };
    vk::PipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_TRUE,
        
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,

        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,

        .colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo color_blending = { .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &color_blend_attachment };

    std::vector dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamic_state = { .dynamicStateCount = (u32)dynamic_states.size(), .pDynamicStates = dynamic_states.data() };

    std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
    auto create_layout_bindings = [&](Shader::ShaderData data) {
        for (auto& buf_info : data.buffers) {
            switch (buf_info.desc_info.type) {
            case Shader::DescriptorType::Vsharp: {
                layout_bindings.push_back(vk::DescriptorSetLayoutBinding(buf_info.binding, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr));
                break;
            }

            case Shader::DescriptorType::Tsharp: {
                layout_bindings.push_back(vk::DescriptorSetLayoutBinding(buf_info.binding, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics, nullptr));
                break;
            }
            }
        }
    };

    create_layout_bindings(vert_shader);
    create_layout_bindings(pixel_shader);

    // Create the descriptor set layout
    vk::DescriptorSetLayoutCreateInfo layout_info = {
        .flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR,
        .bindingCount = (u32)layout_bindings.size(),
        .pBindings = layout_bindings.data()
    };
    descriptor_set_layout = vk::raii::DescriptorSetLayout(device, layout_info);

    // Create the pipeline layout
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
}

std::vector<Pipeline::VertexBinding>* Pipeline::gatherVertices(u32 cnt) {
    // Create a new vertex binding array and initialize it with the fetch shader bindings
    auto& new_vtx_bindings = vtx_bindings.emplace_back();
    for (auto& vtx_binding_layout_element : vtx_binding_layout) {
        auto& binding = new_vtx_bindings.emplace_back();
        binding.fetch_shader_binding = vtx_binding_layout_element;
    }

    // Create Vulkan buffers and copy data
    for (auto& vtx_binding : new_vtx_bindings) {
        // Get pointer to the V#
        VSharp* vsharp = vtx_binding.fetch_shader_binding.vsharp_loc.asPtr();
        // Setup vertex buffer and copy data
        const auto buf_size = (vsharp->stride == 0 ? 1 : vsharp->stride) * cnt;
        vtx_binding.buf = vk::raii::Buffer(device, { .size = buf_size, .usage = vk::BufferUsageFlagBits::eVertexBuffer, .sharingMode = vk::SharingMode::eExclusive });
        auto mem_requirements = vtx_binding.buf.getMemoryRequirements();
        vtx_binding.mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
        vtx_binding.buf.bindMemory(*vtx_binding.mem, 0);
        void* vtx_buf_data = vtx_binding.mem.mapMemory(0, buf_size);
        void* guest_vtx_buf_data = (void*)(vsharp->base + vtx_binding.fetch_shader_binding.voffs);
        std::memcpy(vtx_buf_data, guest_vtx_buf_data, buf_size);
        vtx_binding.mem.unmapMemory();
    }

    return &new_vtx_bindings;
}

std::vector<vk::WriteDescriptorSet> Pipeline::uploadBuffersAndTextures() {
    // Create and upload buffers required by each shader
    std::vector<vk::WriteDescriptorSet> descriptor_writes;
    
    auto create_buffers = [&](Shader::ShaderData data) {
        for (auto& buf_info : data.buffers) {
            switch (buf_info.desc_info.type) {
            case Shader::DescriptorType::Vsharp: {
                // Get pointer to the V#
                VSharp* vsharp = buf_info.desc_info.asPtr<VSharp>();

                // Upload as SSBO
                auto& buf = bufs.emplace_back(nullptr);
                auto& mem = bufs_mem.emplace_back(nullptr);

                const auto buf_size = (vsharp->stride == 0 ? 1 : vsharp->stride) * vsharp->num_records;
                buf = vk::raii::Buffer(device, { .size = buf_size, .usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive });
                auto mem_requirements = buf.getMemoryRequirements();
                mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
                buf.bindMemory(*mem, 0);
                void* buf_data = mem.mapMemory(0, buf_size);
                void* guest_buf_data = (void*)vsharp->base;
                std::memcpy(buf_data, guest_buf_data, buf_size);
                mem.unmapMemory();
                
                buffer_info.push_back({
                    .buffer = *buf,
                    .offset = 0,
                    .range = buf_size,
                });
                descriptor_writes.push_back(vk::WriteDescriptorSet{
                    .dstSet = nullptr,  // Not used for push descriptors
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
                vk::DescriptorImageInfo* image_info;
                Vulkan::getVulkanImageInfoForTSharp(tsharp, &image_info);
                if (image_info == nullptr) break;

                descriptor_writes.push_back(vk::WriteDescriptorSet{
                    .dstSet = nullptr,  // Not used for push descriptors
                    .dstBinding = (u32)buf_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = image_info
                });
                break;
            }
            }
        }
        };

    create_buffers(vert_shader);
    create_buffers(pixel_shader);
    return descriptor_writes;
}

void Pipeline::clearBuffers() {
    vtx_bindings.clear();
    bufs.clear();
    bufs_mem.clear();
    buffer_info.clear();
}

vk::raii::ShaderModule Pipeline::createShaderModule(const std::vector<u32>& code) {
    vk::ShaderModuleCreateInfo create_info = { .codeSize = code.size() * sizeof(u32), .pCode = code.data() };
    vk::raii::ShaderModule shader_module = { device, create_info };

    return shader_module;
}

}   // End namespace PS4::GCN::Vulkan