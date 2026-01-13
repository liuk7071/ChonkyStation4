#pragma once

#include <Common.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/FetchShader.hpp>
#include <deque>
#include "vk_mem_alloc.h"


class VSharp;

namespace PS4::GCN::Vulkan {

class Pipeline {
public:
    Pipeline(Shader::ShaderData vert_shader, Shader::ShaderData pixel_shader, FetchShader fetch_shader);
    Shader::ShaderData vert_shader;
    Shader::ShaderData pixel_shader;
    FetchShader fetch_shader;

    struct VertexBinding {
        FetchShaderVertexBinding fetch_shader_binding;
        vk::Buffer buf = nullptr;
        VmaAllocation alloc;
    };

    vk::raii::Pipeline& getVkPipeline() {
        return graphics_pipeline;
    }
    vk::raii::PipelineLayout& getVkPipelineLayout() {
        return pipeline_layout;
    }

    std::vector<VertexBinding>* gatherVertices();
    std::vector<vk::WriteDescriptorSet> uploadBuffersAndTextures();
    void clearBuffers();

private:
    vk::raii::PipelineLayout pipeline_layout = nullptr;
    vk::raii::Pipeline graphics_pipeline = nullptr;
    vk::raii::DescriptorSetLayout descriptor_set_layout = nullptr;

    std::vector<FetchShaderVertexBinding> vtx_binding_layout;
    // Each time we gather vertices, create a copy of the above vertex binding "layout"
    // The VertexBinding struct is basically FetchShaderVertexBinding with Vulkan buffers added on top.
    // The Vulkan buffers will be populated every time gatherVertices is called and added to this vector.
    std::deque<std::vector<VertexBinding>> vtx_bindings;
    std::vector<vk::Buffer> bufs;
    std::vector<VmaAllocation> buf_allocs;
    std::deque<vk::DescriptorBufferInfo> buffer_info;

    vk::raii::ShaderModule createShaderModule(const std::vector<u32>& code);
};

}   // End namespace PS4::GCN::Vulkan