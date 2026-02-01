#include "PipelineCache.hpp"
#include <Logger.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/RegisterOffsets.hpp>
#include <unordered_map>
#include <memory>
#include <xxhash.h>


namespace PS4::GCN::Vulkan::PipelineCache {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

std::unordered_map<u64, Pipeline*> pipelines;
std::unordered_map<u64, Shader::ShaderData*> shaders;

Pipeline& getPipeline(const u8* vert_shader_code, const u8* pixel_shader_code, const u8* fetch_shader_code, const u32* regs) {
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
    
    PipelineConfig cfg;
    cfg.vertex_hash = vert_shader.hash;
    cfg.pixel_hash = pixel_shader.hash;

    // Hash fetch shader V#s
    XXH3_state_t* state = XXH3_createState();
    XXH3_64bits_reset(state);
    int index = 0;
    for (auto& binding : fetch_shader.bindings) {
        auto* vsharp = binding.vsharp_loc.asPtr();
        const u64 stride = vsharp->stride;
        const u64 nfmt   = vsharp->nfmt;
        const u64 dfmt   = vsharp->dfmt;
        XXH3_64bits_update(state, &index, sizeof(index));
        XXH3_64bits_update(state, &stride, sizeof(stride));
        XXH3_64bits_update(state, &nfmt, sizeof(nfmt));
        XXH3_64bits_update(state, &dfmt, sizeof(dfmt));
    }
    cfg.binding_hash = XXH3_64bits_digest(state);

    // Primitive info
    cfg.prim_type = regs[Reg::mmVGT_PRIMITIVE_TYPE__CI__VI];

    // Color blending info
    for (int i = 0; i < 8; i++) {
        cfg.blend_control[i].raw = regs[Reg::mmCB_BLEND0_CONTROL + i];
    }

    // Depth control
    cfg.depth_control.raw = regs[Reg::mmDB_DEPTH_CONTROL];
    cfg.max_depth_bounds = reinterpret_cast<const float&>(regs[Reg::mmDB_DEPTH_BOUNDS_MAX]);
    cfg.min_depth_bounds = reinterpret_cast<const float&>(regs[Reg::mmDB_DEPTH_BOUNDS_MIN]);

    // Depth clamp
    cfg.enable_depth_clamp = (regs[Reg::mmDB_RENDER_OVERRIDE] >> 16) != 1;   // DISABLE_VIEWPORT_CLAMP

    // Calculate final pipeline hash
    const u64 pipeline_hash = XXH3_64bits(&cfg, sizeof(PipelineConfig));
    if (pipelines.contains(pipeline_hash))
        return *pipelines[pipeline_hash];

    log("Compiling new pipeline\n");
    auto* pipeline = new Pipeline(vert_shader, pixel_shader, fetch_shader, cfg);
    pipelines[pipeline_hash] = pipeline;
    return *pipeline;
}

}   // End namespace PS4::GCN::Vulkan::PipelineCache