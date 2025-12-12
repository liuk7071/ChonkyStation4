#include "PipelineCache.hpp"
#include <GCN/FetchShader.hpp>


namespace PS4::GCN::Vulkan::PipelineCache {

std::deque<Pipeline> pipelines;

Pipeline& getPipeline(const u8* vert_shader_code, const u8* pixel_shader_code, const u8* fetch_shader_code) {
    // Compile shaders
    // TODO: Shader cache
    GCN::FetchShader fetch_shader = FetchShader(fetch_shader_code);
    Shader::ShaderData vert_shader, pixel_shader;
    Shader::decompileShader((u32*)vert_shader_code, Shader::ShaderStage::Vertex, vert_shader, &fetch_shader);
    Shader::decompileShader((u32*)pixel_shader_code, Shader::ShaderStage::Fragment, pixel_shader);

    
    // TODO: When we have more state to check (i.e. render targets), switch to a hashing algorithm + unordered_map

    for (auto& pipeline : pipelines) {
        if (pipeline.vert_shader == vert_shader && pipeline.pixel_shader == pixel_shader && pipeline.fetch_shader == fetch_shader) {
            return pipeline;
        }
    }

    auto& pipeline = pipelines.emplace_back(vert_shader, pixel_shader, fetch_shader);
    return pipeline;
}

}   // End namespace PS4::GCN::Vulkan::PipelineCache