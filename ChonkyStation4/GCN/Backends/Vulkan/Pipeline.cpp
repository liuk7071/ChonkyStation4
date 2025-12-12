#include "Pipeline.hpp"
#include <Logger.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>


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
        attrib = { shader_binding.dest_vgpr, n_binding++, getVtxBufferFormat(shader_binding.n_elements, 0 /* TODO: type */), 0 };

        auto& vtx_binding = vtx_bindings.emplace_back();
        vtx_binding.fetch_shader_binding = shader_binding;
        log("Created attribute binding for location %d\n", shader_binding.dest_vgpr);
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

void Pipeline::gatherVertices(u32 cnt) {
    for (auto& vtx_binding : vtx_bindings) {
        // Get pointer to the V#
        VSharp* vsharp = vtx_binding.fetch_shader_binding.vsharp_loc.asPtr();
        // Setup vertex buffer and copy data
        const auto buf_size = vsharp->stride * cnt;
        vtx_binding.buf = vk::raii::Buffer(device, { .size = buf_size, .usage = vk::BufferUsageFlagBits::eVertexBuffer, .sharingMode = vk::SharingMode::eExclusive });
        auto mem_requirements = vtx_binding.buf.getMemoryRequirements();
        vtx_binding.mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
        vtx_binding.buf.bindMemory(*vtx_binding.mem, 0);
        void* vtx_buf_data = vtx_binding.mem.mapMemory(0, buf_size);
        void* guest_vtx_buf_data = (void*)(vsharp->base + vtx_binding.fetch_shader_binding.voffs);
        std::memcpy(vtx_buf_data, guest_vtx_buf_data, buf_size);
        vtx_binding.mem.unmapMemory();
    }
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
                void* guest_buf_data = (void*)(vsharp->base << 8);
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

                //std::ofstream out;
                //out.open(std::format("{}.bin", (tsharp->base_address << 8)), std::ios::binary);
                //out.write((char*)(tsharp->base_address << 8), img_size);

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

                image_info.push_back({
                    .sampler = *sampler,
                    .imageView = *img_view,
                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                    });
                descriptor_writes.push_back(vk::WriteDescriptorSet{
                    .dstSet = nullptr,  // Not used for push descriptors
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
    return descriptor_writes;
}

void Pipeline::clearBuffers() {
    bufs.clear();
    bufs_mem.clear();
    textures.clear();
    texture_mem.clear();
    texture_samplers.clear();
    texture_views.clear();
    buffer_info.clear();
    image_info.clear();
}

vk::raii::ShaderModule Pipeline::createShaderModule(const std::vector<u32>& code) {
    vk::ShaderModuleCreateInfo create_info = { .codeSize = code.size() * sizeof(u32), .pCode = code.data() };
    vk::raii::ShaderModule shader_module = { device, create_info };

    return shader_module;
}

vk::Format Pipeline::getVtxBufferFormat(u32 n_elements, u32 type) {
    // TODO: for now type is ignored
    switch (n_elements) {
    case 2:     return vk::Format::eR32G32Sfloat;
    case 3:     return vk::Format::eR32G32B32Sfloat;
    case 4:     return vk::Format::eR32G32B32A32Sfloat;
    default:    Helpers::panic("Vulkan: getVtxBuffeFormat unhandled n_elements=%d\n", n_elements);
    }
}

// Returns a Vulkan format alongside the size of 1 pixel in bytes
std::pair<vk::Format, size_t> Pipeline::getTexFormatAndSize(u32 dfmt, u32 nfmt) {
    switch ((DataFormat)dfmt) {

    case DataFormat::Format8_8_8_8: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm: return { vk::Format::eR8G8B8A8Unorm, sizeof(u32) };

        default:    Helpers::panic("Unimplemented texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    case DataFormat::Format5_6_5: {
        switch ((NumberFormat)nfmt) {

        case NumberFormat::Unorm: return { vk::Format::eR5G6B5UnormPack16, sizeof(u16) };

        default:    Helpers::panic("Unimplemented texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
        }
        break;
    }

    default:    Helpers::panic("Unimplemented texture format: dfmt=%d, nfmt=%d\n", dfmt, nfmt);
    }

    Helpers::panic("getTexFormatAndSize: unreachable\n");
}

}   // End namespace PS4::GCN::Vulkan