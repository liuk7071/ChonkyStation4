#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/Backends/Vulkan/ShaderCache.hpp>
#include <deque>
#include "vk_mem_alloc.h"


class VSharp;

namespace PS4::GCN::Vulkan {

struct TrackedTexture;

class ComputePipeline {
public:
    ComputePipeline(ShaderCache::CachedShader* compute_shader);
    ShaderCache::CachedShader* compute_shader;

    struct VertexBinding {
        FetchShaderVertexBinding fetch_shader_binding;
        vk::Buffer buf = nullptr;
        size_t offs_in_buf = 0;
        VmaAllocation alloc;
    };

    struct PushConstants {
        u16 stride[48];
        u32 fmt[32];    // TODO
    };

    vk::raii::Pipeline& getVkPipeline() {
        return compute_pipeline;
    }
    vk::raii::PipelineLayout& getVkPipelineLayout() {
        return pipeline_layout;
    }

    std::vector<vk::WriteDescriptorSet> uploadBuffersAndTextures(PushConstants** push_constants_ptr, TrackedTexture* rt, bool* has_feedback_loop);
    void clearBuffers();

private:
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline compute_pipeline = nullptr;
    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;

    std::deque<vk::DescriptorBufferInfo> buffer_info;
    PushConstants push_constants;
};

}   // End namespace PS4::GCN::Vulkan