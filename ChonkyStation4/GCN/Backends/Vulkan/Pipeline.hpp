#pragma once

#include <Common.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/FetchShader.hpp>
#include <deque>


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
        vk::raii::Buffer buf = nullptr;
        vk::raii::DeviceMemory mem = nullptr;
    };

    vk::raii::Pipeline& getVkPipeline() {
        return graphics_pipeline;
    }
    vk::raii::PipelineLayout& getVkPipelineLayout() {
        return pipeline_layout;
    }

    std::vector<VertexBinding>* gatherVertices(u32 cnt);
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
    std::vector<vk::raii::Buffer> bufs;
    std::vector<vk::raii::DeviceMemory> bufs_mem;
    std::vector<vk::raii::Image> textures;
    std::vector<vk::raii::DeviceMemory> texture_mem;
    std::vector<vk::raii::ImageView> texture_views;
    std::vector<vk::raii::Sampler> texture_samplers;
    std::vector<vk::DescriptorBufferInfo> buffer_info;
    std::vector<vk::DescriptorImageInfo> image_info;

    vk::raii::ShaderModule createShaderModule(const std::vector<u32>& code);
    vk::Format getVtxBufferFormat(u32 n_elements, u32 type);
    std::pair<vk::Format, size_t> getTexFormatAndSize(u32 dfmt, u32 nfmt);
};

}   // End namespace PS4::GCN::Vulkan