#include "ShaderCache.hpp"
#include <Logger.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>
#include <xxhash.h>
#include <unordered_map>


namespace PS4::GCN::Vulkan::ShaderCache {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

std::unordered_map<u64, CachedShader*> shaders;

CachedShader* getShader(const u8* code, Shader::ShaderStage stage, FetchShader* fetch_shader, ComputeJob* compute_job) {
    // Find shader header
    u32* ptr = (u32*)code;
    while (*ptr != 0x5362724F) {    // "OrbS"
        ptr++;
    }

    // Get the shader hash from the header
    u64 hash;
    ptr += 4;
    std::memcpy(&hash, ptr, sizeof(u64));

    // Hash compute job info if this is a compute shader
    if (stage == Shader::ShaderStage::Compute) {
        XXH3_state_t* state = XXH3_createState();
        XXH3_64bits_reset(state);

        XXH3_64bits_update(state, &hash, sizeof(hash));
        XXH3_64bits_update(state, &compute_job->n_threads_x, sizeof(compute_job->n_threads_x));
        XXH3_64bits_update(state, &compute_job->n_threads_y, sizeof(compute_job->n_threads_y));
        XXH3_64bits_update(state, &compute_job->n_threads_z, sizeof(compute_job->n_threads_z));

        hash = XXH3_64bits_digest(state);
        XXH3_freeState(state);
    }

    // Check if this shader was cached, otherwise compile and cache it
    if (shaders.contains(hash)) {
        return shaders[hash];
    }

    auto shader_stage = [](Shader::ShaderStage stage) -> EShLanguage {
        switch (stage) {
        case Shader::ShaderStage::Vertex:       return EShLangVertex;
        case Shader::ShaderStage::Fragment:     return EShLangFragment;
        case Shader::ShaderStage::Compute:      return EShLangCompute;
        default: Helpers::panic("shader_stage: unreachable");
        }
    };

    // Compile it
    log("Compiling new shader %016llx\n", hash);
    CachedShader* cached_shader = new CachedShader();
    Shader::decompileShader((u32*)code, stage, cached_shader->data, fetch_shader, compute_job);
    cached_shader->vk_shader = createShaderModule(GCN::compileGLSL(cached_shader->data.source, shader_stage(stage)));

    // Cache it
    cached_shader->data.hash = hash;
    shaders[hash] = cached_shader;
    return cached_shader;
}

}   // End namespace PS4::GCN::Vulkan::ShaderCache