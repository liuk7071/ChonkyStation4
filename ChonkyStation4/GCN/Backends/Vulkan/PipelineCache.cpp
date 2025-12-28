#include "PipelineCache.hpp"
#include <Logger.hpp>
#include <GCN/FetchShader.hpp>
#include <unordered_map>
#include <memory>


namespace PS4::GCN::Vulkan::PipelineCache {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

std::deque<Pipeline> pipelines;
std::unordered_map<u64, Shader::ShaderData*> shaders;

Pipeline& getPipeline(const u8* vert_shader_code, const u8* pixel_shader_code, const u8* fetch_shader_code) {
    // Compile shaders
    GCN::FetchShader fetch_shader = FetchShader(fetch_shader_code);
    Shader::ShaderData vert_shader, pixel_shader;

    auto shader_cache = [&](const u8* code, Shader::ShaderStage stage) -> Shader::ShaderData* {
        // Find shader header
        u32* ptr = (u32*)code;
        while (*ptr != 0x5362724F) {    // "OrbS"
            ptr++;
        }

        // Get the shader hash from the header
        u64 hash;
        ptr += 4;
        std::memcpy(&hash, ptr, sizeof(u64));
        
        // Check if this shader was cached, otherwise compile and cache it
        if (shaders.contains(hash)) {
            return shaders[hash];
        }

        // Compile it
        log("Compiling new shader %016llx\n", hash);
        Shader::ShaderData* shader_data = new Shader::ShaderData();
        Shader::decompileShader((u32*)code, stage, *shader_data, &fetch_shader);
        // Cache it
        shader_data->hash = hash;
        shaders[hash] = shader_data;
        return shader_data;
    };

    vert_shader  = *shader_cache(vert_shader_code, Shader::ShaderStage::Vertex);
    pixel_shader = *shader_cache(pixel_shader_code, Shader::ShaderStage::Fragment);
    
    // TODO: When we have more state to check (i.e. render targets), switch to a hashing algorithm + unordered_map

    for (auto& pipeline : pipelines) {
        if (pipeline.vert_shader.hash == vert_shader.hash && pipeline.pixel_shader.hash == pixel_shader.hash && pipeline.fetch_shader == fetch_shader) {
            return pipeline;
        }
    }

    log("Compiling new pipeline\n");
    auto& pipeline = pipelines.emplace_back(vert_shader, pixel_shader, fetch_shader);
    return pipeline;
}

}   // End namespace PS4::GCN::Vulkan::PipelineCache