#include "Pipeline.hpp"
#include <Logger.hpp>
#include <GCN/HostTessShaders.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/BufferCache.hpp>
#include <GCN/Backends/Vulkan/TextureCache.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Detiler/gpuaddr.h>
#include <GCN/Detiler/gnm/texture.h>


namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

Pipeline::Pipeline(ShaderCache::CachedShader* vert_shader, ShaderCache::CachedShader* pixel_shader, FetchShader fetch_shader, PipelineConfig& cfg) : vert_shader(vert_shader), pixel_shader(pixel_shader), fetch_shader(fetch_shader), cfg(cfg) {
    // Iterate over fetch shader bindings and convert them to vulkan binding/attribute descriptions, and create the vertex buffers
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attribs;
    u32 n_binding = 0;
    for (auto& shader_binding : fetch_shader.bindings) {
        // Get pointer to the V#
        VSharp* vsharp = shader_binding.vsharp_loc.asPtr();
        auto& binding = bindings.emplace_back();
        auto& attrib = attribs.emplace_back();

        binding = { n_binding, (u32)vsharp->stride, vk::VertexInputRate::eVertex };
        attrib = { shader_binding.idx, n_binding++, getBufFormatAndSize(vsharp->dfmt, vsharp->nfmt).first, 0 };

        auto& vtx_binding = vtx_binding_layout.emplace_back();
        vtx_binding = shader_binding;
        log("Created attribute binding for location %d\n", shader_binding.idx);
    }

    // Setup graphics pipeline
    if (!cfg.has_vs)
        Helpers::panic("TODO: no vertex shader");

    int stage_count = 1;
    vk::PipelineShaderStageCreateInfo vert_stage_info = { .stage = vk::ShaderStageFlagBits::eVertex, .module = vert_shader->vk_shader, .pName = "main" };
    vk::PipelineShaderStageCreateInfo shader_stages[4] = { vert_stage_info };

    if (cfg.has_ps) {
        shader_stages[stage_count++] = { .stage = vk::ShaderStageFlagBits::eFragment, .module = pixel_shader->vk_shader, .pName = "main"};
    }
    
    vk::PipelineTessellationStateCreateInfo tess;
    if (cfg.prim_type == (u32)PrimitiveType::RectList || cfg.prim_type == (u32)PrimitiveType::QuadList) {
        std::string tcs;
        std::string tes;

        if (cfg.prim_type == (u32)PrimitiveType::RectList) {
            tess.patchControlPoints = 3;
            HostShaders::generateRectTCS(&vert_shader->data, tcs);
            HostShaders::generateRectTES(&vert_shader->data, tes);
        }
        else {
            tess.patchControlPoints = 4;
            HostShaders::generateQuadTCS(&vert_shader->data, tcs);
            HostShaders::generateQuadTES(&vert_shader->data, tes);
        }

        tess_control_shader = createShaderModule(GCN::compileGLSL(tcs, EShLangTessControl));
        tess_eval_shader    = createShaderModule(GCN::compileGLSL(tes, EShLangTessEvaluation));
        shader_stages[stage_count++] = { .stage = vk::ShaderStageFlagBits::eTessellationControl,    .module = tess_control_shader,  .pName = "main" };
        shader_stages[stage_count++] = { .stage = vk::ShaderStageFlagBits::eTessellationEvaluation, .module = tess_eval_shader,     .pName = "main" };
    }

    vk::PipelineVertexInputStateCreateInfo vertex_input_info = { .vertexBindingDescriptionCount = (u32)bindings.size(), .pVertexBindingDescriptions = bindings.data(), .vertexAttributeDescriptionCount = (u32)attribs.size(), .pVertexAttributeDescriptions = attribs.data() };
    
    auto topology = [](u32 prim_type) {
        switch ((PrimitiveType)prim_type) {
        case PrimitiveType::PointList:          return vk::PrimitiveTopology::ePointList;
        case PrimitiveType::LineList:           return vk::PrimitiveTopology::eLineList;
        case PrimitiveType::LineStrip:          return vk::PrimitiveTopology::eLineStrip;
        case PrimitiveType::TriangleList:       return vk::PrimitiveTopology::eTriangleList;
        case PrimitiveType::TriangleFan:        return vk::PrimitiveTopology::eTriangleFan;
        case PrimitiveType::TriangleStrip:      return vk::PrimitiveTopology::eTriangleStrip;
        case PrimitiveType::AdjLineList:        return vk::PrimitiveTopology::eLineListWithAdjacency;
        case PrimitiveType::AdjLineStrip:       return vk::PrimitiveTopology::eLineStripWithAdjacency;
        case PrimitiveType::AdjTriangleList:    return vk::PrimitiveTopology::eTriangleListWithAdjacency;
        case PrimitiveType::AdjTriangleStrip:   return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
        
        // Rects and Quads are implemented using tessellation shaders
        case PrimitiveType::RectList:           return vk::PrimitiveTopology::ePatchList;
        case PrimitiveType::QuadList:           return vk::PrimitiveTopology::ePatchList;
        default:    Helpers::panic("Unimplemented primitive type %d\n", prim_type);
        }
    };
    
    vk::PipelineInputAssemblyStateCreateInfo input_assembly = { .topology = topology(cfg.prim_type) };

    // Viewport
    const auto z_offset = cfg.viewport_control.z_offset_enable ? cfg.z_offset : 0.0f;
    const auto z_scale = cfg.viewport_control.z_scale_enable ? cfg.z_scale : 1.0f;
    if (!cfg.dx_clip_space_enable) {
        // -1 ... +1
        min_viewport_depth = z_offset - z_scale;
        max_viewport_depth = z_offset + z_scale;
    }
    else {
        // 0 ... 1
        min_viewport_depth = z_offset;
        max_viewport_depth = z_offset + z_scale;
    }

    vk::PipelineViewportDepthClipControlCreateInfoEXT depth_clip_control = {
        .negativeOneToOne = !cfg.dx_clip_space_enable
    };

    vk::PipelineViewportStateCreateInfo viewport_state = { 
        .viewportCount = 1,
        .scissorCount = 1,
        .pNext = &depth_clip_control
    };
    
    vk::PipelineRasterizationStateCreateInfo rasterizer = {
        .depthClampEnable = cfg.enable_depth_clamp,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f
    };
    vk::PipelineMultisampleStateCreateInfo multisampling = { .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };
    
    
    auto compare_op = [](u32 func) -> vk::CompareOp {
        switch ((CompareFunc)func) {
        case CompareFunc::Never:        return vk::CompareOp::eNever;
        case CompareFunc::Less:         return vk::CompareOp::eLess;
        case CompareFunc::Equal:        return vk::CompareOp::eEqual;
        case CompareFunc::LessEqual:    return vk::CompareOp::eLessOrEqual;
        case CompareFunc::Greater:      return vk::CompareOp::eGreater;
        case CompareFunc::NotEqual:     return vk::CompareOp::eNotEqual;
        case CompareFunc::GreaterEqual: return vk::CompareOp::eGreaterOrEqual;
        case CompareFunc::Always:       return vk::CompareOp::eAlways;
        }
    };

    auto stencil_op = [](u32 op, StencilRefMask refmask) -> vk::StencilOp {
        auto assert_opval = [&]() {
            if (refmask.stencil_ref != refmask.stencil_op_val) {
                //Helpers::panic("Stencil: ref != op_val");
                printf("Stencil: ref != op_val\n");
            }
        };

        switch ((StencilOp)op) {
        case StencilOp::Keep:                               return vk::StencilOp::eKeep;
        case StencilOp::Zero:                               return vk::StencilOp::eZero;
        case StencilOp::ReplaceTest:                        return vk::StencilOp::eReplace;
        case StencilOp::ReplaceOpVal:   assert_opval();     return vk::StencilOp::eReplace;
        case StencilOp::AddOpValClamp:  assert_opval();     return vk::StencilOp::eIncrementAndClamp;
        case StencilOp::SubOpValClamp:  assert_opval();     return vk::StencilOp::eDecrementAndClamp;
        case StencilOp::Invert:                             return vk::StencilOp::eInvert;
        case StencilOp::AddOpValWrap:   assert_opval();     return vk::StencilOp::eIncrementAndWrap;
        case StencilOp::SubOpValWrap:   assert_opval();     return vk::StencilOp::eDecrementAndWrap;
        default: Helpers::panic("Unimplemented stencil op %d\n", op);
        }
    };

    vk::StencilOpState stencil_front;
    vk::StencilOpState stencil_back;
    stencil_front.failOp      = stencil_op(cfg.stencil_control.front_fail_op,       cfg.stencil_refmask_front);
    stencil_front.passOp      = stencil_op(cfg.stencil_control.front_pass_op,       cfg.stencil_refmask_front);
    stencil_front.depthFailOp = stencil_op(cfg.stencil_control.front_depth_fail_op, cfg.stencil_refmask_front);
    stencil_front.compareOp   = compare_op(cfg.depth_control.stencil_func_front);
    stencil_front.compareMask = cfg.stencil_refmask_front.stencil_compare_mask;
    stencil_front.writeMask   = cfg.stencil_refmask_front.stencil_write_mask;
    stencil_front.reference   = cfg.stencil_refmask_front.stencil_ref;
    if (cfg.depth_control.stencil_backface_enable) {
        stencil_back.failOp         = stencil_op(cfg.stencil_control.back_fail_op,       cfg.stencil_refmask_back);
        stencil_back.passOp         = stencil_op(cfg.stencil_control.back_pass_op,       cfg.stencil_refmask_back);
        stencil_back.depthFailOp    = stencil_op(cfg.stencil_control.back_depth_fail_op, cfg.stencil_refmask_back);
        stencil_back.compareOp      = compare_op(cfg.depth_control.stencil_func_back);
        stencil_back.compareMask    = cfg.stencil_refmask_back.stencil_compare_mask;
        stencil_back.writeMask      = cfg.stencil_refmask_back.stencil_write_mask;
        stencil_back.reference      = cfg.stencil_refmask_back.stencil_ref;
    }
    else {
        stencil_back = stencil_front;
    }

    vk::PipelineDepthStencilStateCreateInfo depth_stencil = {
        .depthTestEnable        = cfg.depth_control.depth_enable,
        .depthWriteEnable       = cfg.depth_control.depth_write_enable,
        .depthCompareOp         = compare_op(cfg.depth_control.depth_func),
        .depthBoundsTestEnable  = cfg.depth_control.depth_bounds_enable,
        .maxDepthBounds         = cfg.max_depth_bounds,
        .minDepthBounds         = cfg.min_depth_bounds,
        .stencilTestEnable      = cfg.depth_control.stencil_enable,
        .front                  = stencil_front,
        .back                   = stencil_back
    };

    auto blend_factor = [](u32 factor) -> vk::BlendFactor {
        switch ((BlendFactor)factor) {
        case BlendFactor::Zero:                     return vk::BlendFactor::eZero;
        case BlendFactor::One:                      return vk::BlendFactor::eOne;
        case BlendFactor::SrcColor:                 return vk::BlendFactor::eSrcColor;
        case BlendFactor::OneMinusSrcColor:         return vk::BlendFactor::eOneMinusSrcColor;
        case BlendFactor::SrcAlpha:                 return vk::BlendFactor::eSrcAlpha;
        case BlendFactor::OneMinusSrcAlpha:         return vk::BlendFactor::eOneMinusSrcAlpha;
        case BlendFactor::DstAlpha:                 return vk::BlendFactor::eDstAlpha;
        case BlendFactor::OneMinusDstAlpha:         return vk::BlendFactor::eOneMinusDstAlpha;
        case BlendFactor::DstColor:                 return vk::BlendFactor::eDstColor;
        case BlendFactor::OneMinusDstColor:         return vk::BlendFactor::eOneMinusDstColor;
        case BlendFactor::SrcAlphaSaturate:         return vk::BlendFactor::eSrcAlphaSaturate;
        //case BlendFactor::ConstantColor:            return vk::BlendFactor::eConstantColor;
        //case BlendFactor::OneMinusConstantColor:    return vk::BlendFactor::eOneMinusConstantColor;
        case BlendFactor::Src1Color:                return vk::BlendFactor::eSrc1Color;
        case BlendFactor::InvSrc1Color:             return vk::BlendFactor::eOneMinusSrc1Color;
        case BlendFactor::Src1Alpha:                return vk::BlendFactor::eSrc1Alpha;
        case BlendFactor::InvSrc1Alpha:             return vk::BlendFactor::eOneMinusSrc1Alpha;
        //case BlendFactor::ConstantAlpha:            return vk::BlendFactor::eConstantAlpha;
        //case BlendFactor::OneMinusConstantAlpha:    return vk::BlendFactor::eOneMinusConstantAlpha;
        default:    Helpers::panic("Unimplemented blend factor %d\n", factor);
        }
    };

    auto blend_op = [](u32 func) -> vk::BlendOp {
        switch ((BlendFunc)func) {
        case BlendFunc::Add:                return vk::BlendOp::eAdd;
        case BlendFunc::Subtract:           return vk::BlendOp::eSubtract;
        case BlendFunc::Min:                return vk::BlendOp::eMin;
        case BlendFunc::Max:                return vk::BlendOp::eMax;
        case BlendFunc::ReverseSubtract:    return vk::BlendOp::eReverseSubtract;
        default:    Helpers::panic("Unknown blend func %d\n", func);
        }
    };

    auto color_src_blend = blend_factor(cfg.blend_control[0].src_blend);
    auto color_dst_blend = blend_factor(cfg.blend_control[0].dst_blend);
    auto alpha_src_blend = cfg.blend_control[0].separate_alpha_blend ? blend_factor(cfg.blend_control[0].alpha_src_blend) : color_src_blend;
    auto alpha_dst_blend = cfg.blend_control[0].separate_alpha_blend ? blend_factor(cfg.blend_control[0].alpha_dst_blend) : color_dst_blend;
    
    auto color_op = blend_op(cfg.blend_control[0].color_func);
    auto alpha_op = cfg.blend_control[0].separate_alpha_blend ? blend_op(cfg.blend_control[0].alpha_func) : color_op;

    vk::PipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = cfg.blend_control[0].enable,
        
        .srcColorBlendFactor = color_src_blend,
        .dstColorBlendFactor = color_dst_blend,
        .colorBlendOp = color_op,

        .srcAlphaBlendFactor = alpha_src_blend,
        .dstAlphaBlendFactor = alpha_dst_blend,
        .alphaBlendOp = alpha_op,

        .colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo color_blending = { .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &color_blend_attachment };

    std::vector dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eAttachmentFeedbackLoopEnableEXT };
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

    create_layout_bindings(vert_shader->data);
    
    if (cfg.has_ps)
        create_layout_bindings(pixel_shader->data);

    // Create the descriptor set layout
    vk::DescriptorSetLayoutCreateInfo layout_info = {
        .flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR,
        .bindingCount = (u32)layout_bindings.size(),
        .pBindings = layout_bindings.data()
    };
    descriptor_set_layout = vk::raii::DescriptorSetLayout(device, layout_info);

    // Create the pipeline layout
    vk::PushConstantRange push_constant_range = {
        vk::ShaderStageFlagBits::eAllGraphics,
        0,
        sizeof(PushConstants)
    };
    vk::PipelineLayoutCreateInfo pipeline_layout_info = { .setLayoutCount = 1, .pSetLayouts = &*descriptor_set_layout, .pushConstantRangeCount = 1, .pPushConstantRanges = &push_constant_range };
    pipeline_layout = vk::raii::PipelineLayout(device, pipeline_layout_info);

    vk::GraphicsPipelineCreateInfo gpci = {};
    gpci.stageCount = stage_count;
    gpci.pStages = shader_stages;
    gpci.pVertexInputState = &vertex_input_info;
    gpci.pInputAssemblyState = &input_assembly;
    gpci.pViewportState = &viewport_state;
    gpci.pRasterizationState = &rasterizer;
    gpci.pMultisampleState = &multisampling;
    gpci.pDepthStencilState = &depth_stencil;
    gpci.pColorBlendState = &color_blending;
    gpci.pDynamicState = &dynamic_state;
    gpci.layout = *pipeline_layout;
    gpci.renderPass = VK_NULL_HANDLE;

    if (cfg.prim_type == (u32)PrimitiveType::RectList || cfg.prim_type == (u32)PrimitiveType::QuadList) {
        gpci.pTessellationState = &tess;
    }

    vk::PipelineRenderingCreateInfo rendering_info = {};
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachmentFormats = &swapchain_surface_format.format;
    rendering_info.depthAttachmentFormat = vk::Format::eD32SfloatS8Uint;
    rendering_info.stencilAttachmentFormat = vk::Format::eD32SfloatS8Uint;

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipeline_create_info_chain(gpci, rendering_info);
    graphics_pipeline = vk::raii::Pipeline(device, nullptr, pipeline_create_info_chain.get());
}

std::vector<Pipeline::VertexBinding>* Pipeline::gatherVertices() {
    // Create a new vertex binding array and initialize it with the fetch shader bindings
    auto& new_vtx_bindings = vtx_bindings.emplace_back();
    new_vtx_bindings.reserve(32);
    for (auto& vtx_binding_layout_element : vtx_binding_layout) {
        auto& binding = new_vtx_bindings.emplace_back();
        binding.fetch_shader_binding = vtx_binding_layout_element;
    }

    // Create Vulkan buffers and copy data
    for (auto& vtx_binding : new_vtx_bindings) {
        // Get pointer to the V#
        VSharp* vsharp = vtx_binding.fetch_shader_binding.vsharp_loc.asPtr();
        // Setup vertex buffer and copy data
        const auto buf_size = (vsharp->stride == 0 ? 1 : vsharp->stride) * vsharp->num_records;
        void* guest_vtx_buf_data = (void*)(vsharp->base + vtx_binding.fetch_shader_binding.voffs);
        auto [buf, offs, was_dirty] = Cache::getBuffer(guest_vtx_buf_data, buf_size);
        vtx_binding.buf = buf;
        vtx_binding.offs_in_buf = offs;
    }

    return &new_vtx_bindings;
}

std::vector<vk::WriteDescriptorSet> Pipeline::uploadBuffersAndTextures(PushConstants** push_constants_ptr, TrackedTexture* rt, bool* has_feedback_loop) {
    // Create and upload buffers required by each shader
    std::vector<vk::WriteDescriptorSet> descriptor_writes;
    descriptor_writes.reserve(32);
    std::memset(&push_constants, 0, sizeof(PushConstants));

    *has_feedback_loop = false;

    auto create_buffers = [&](Shader::ShaderData& data) {
        for (auto& buf_info : data.buffers) {
            switch (buf_info.desc_info.type) {
            case Shader::DescriptorType::Vsharp: {
                // Get pointer to the V#
                VSharp* vsharp = buf_info.desc_info.asPtr<VSharp>();

                // Upload as SSBO
                const auto buf_size = (vsharp->stride == 0 ? 1 : vsharp->stride) * (vsharp->num_records + 16);
                void* guest_buf_data = (void*)vsharp->base;
                if ((u64)guest_buf_data < 0x10000) continue;
                auto [cached_buf, offs, was_dirty] = Cache::getBuffer(guest_buf_data, buf_size);

                buffer_info.push_back({
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
                    .pBufferInfo = &buffer_info.back()
                });

                // Write push constants for this buffer
                if (buf_info.binding < 48) {
                    push_constants.stride[buf_info.binding] = vsharp->stride;
                    //push_constants.fmt[buf_info.binding]    = vsharp->dfmt | (vsharp->nfmt << 8);
                } else printf("TODO: buf_info.binding >= 48\n");
                break;
            }

            case Shader::DescriptorType::Tsharp: {
                TSharp* tsharp = buf_info.desc_info.asPtr<TSharp>();
                TrackedTexture* tex;
                Vulkan::getVulkanImageInfoForTSharp(tsharp, &tex);
                tex->was_bound = true;

                if (tex == rt) {
                    *has_feedback_loop = true;
                }

                if (tex->curr_layout != vk::ImageLayout::eShaderReadOnlyOptimal) {
                    endRendering();
                    tex->transition(vk::ImageLayout::eShaderReadOnlyOptimal);
                }
                //if (image_info == nullptr) break;

                descriptor_writes.push_back(vk::WriteDescriptorSet {
                    .dstSet = nullptr,  // Not used for push descriptors
                    .dstBinding = (u32)buf_info.binding,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &tex->image_info
                });
                break;
            }
            }
        }
        };

    create_buffers(vert_shader->data);
    
    if (cfg.has_ps)
        create_buffers(pixel_shader->data);

    *push_constants_ptr = &push_constants;
    return descriptor_writes;
}

void Pipeline::clearBuffers() {
    vtx_bindings.clear();
    buffer_info.clear();
}

}   // End namespace PS4::GCN::Vulkan