#include "PipelineCache.hpp"
#include <Logger.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/RegisterOffsets.hpp>
#include <GCN/Backends/Vulkan/ShaderCache.hpp>
#include <unordered_map>
#include <memory>
#include <xxhash.h>


namespace PS4::GCN::Vulkan::PipelineCache {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

std::unordered_map<u64, Pipeline*> pipelines;

Pipeline& getPipeline(const u8* vert_shader_code, const u8* pixel_shader_code, const u8* fetch_shader_code, const u32* regs) {
    // Compile shaders
    GCN::FetchShader fetch_shader = FetchShader(fetch_shader_code);

    ShaderCache::CachedShader* vert_shader;
    ShaderCache::CachedShader* pixel_shader;
    vert_shader  = ShaderCache::getShader(vert_shader_code,  Shader::ShaderStage::Vertex,   &fetch_shader);
    pixel_shader = ShaderCache::getShader(pixel_shader_code, Shader::ShaderStage::Fragment, &fetch_shader);

    PipelineConfig cfg;
    cfg.vertex_hash = vert_shader->data.hash;
    cfg.pixel_hash  = pixel_shader->data.hash;

    // Hash fetch shader V#s
    XXH3_state_t* state = XXH3_createState();
    XXH3_64bits_reset(state);
    int index = 0;
    for (auto& binding : fetch_shader.bindings) {
        auto* vsharp = binding.vsharp_loc.asPtr();
        const u64 stride = vsharp->stride;
        const u64 nfmt = vsharp->nfmt;
        const u64 dfmt = vsharp->dfmt;
        XXH3_64bits_update(state, &index, sizeof(index));
        XXH3_64bits_update(state, &stride, sizeof(stride));
        XXH3_64bits_update(state, &nfmt, sizeof(nfmt));
        XXH3_64bits_update(state, &dfmt, sizeof(dfmt));
        index++;
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

    // Viewport
    cfg.viewport_control.raw = regs[Reg::mmPA_CL_VTE_CNTL];
    cfg.z_offset = reinterpret_cast<const float&>(regs[Reg::mmPA_CL_VPORT_ZOFFSET]);
    cfg.z_scale  = reinterpret_cast<const float&>(regs[Reg::mmPA_CL_VPORT_ZSCALE]);

    // Clip space
    cfg.dx_clip_space_enable = (regs[Reg::mmPA_CL_CLIP_CNTL] >> 19) & 1;

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