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

    PipelineConfig cfg;
    const bool has_vs = vert_shader_code  != nullptr;
    const bool has_ps = pixel_shader_code != nullptr;
    cfg.has_vs = has_vs;
    cfg.has_ps = has_ps;

    ShaderCache::CachedShader* vert_shader  = nullptr;
    ShaderCache::CachedShader* pixel_shader = nullptr;
    
    if (!has_vs)
        Helpers::panic("TODO: no vertex shader");

    if (has_vs) {
        vert_shader = ShaderCache::getShader(vert_shader_code, Shader::ShaderStage::Vertex, &fetch_shader);
        cfg.vertex_hash = vert_shader->data.hash;
    }
    if (has_ps) {
        pixel_shader = ShaderCache::getShader(pixel_shader_code, Shader::ShaderStage::Fragment, &fetch_shader);
        cfg.pixel_hash  = pixel_shader->data.hash;
    }

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

    XXH3_freeState(state);

    // Primitive info
    cfg.prim_type = regs[Reg::mmVGT_PRIMITIVE_TYPE__CI__VI];

    // Color blending info
    for (int i = 0; i < 8; i++) {
        cfg.blend_control[i].raw = regs[Reg::mmCB_BLEND0_CONTROL + i];
    }

    // Color
    cfg.degamma_enable = (regs[Reg::mmCB_COLOR_CONTROL] >> 3) & 1;

    // Depth control
    cfg.depth_control.raw   = regs[Reg::mmDB_DEPTH_CONTROL];
    cfg.depth_clear_enable  = regs[Reg::mmDB_RENDER_CONTROL] & 1;
    cfg.max_depth_bounds    = reinterpret_cast<const float&>(regs[Reg::mmDB_DEPTH_BOUNDS_MAX]);
    cfg.min_depth_bounds    = reinterpret_cast<const float&>(regs[Reg::mmDB_DEPTH_BOUNDS_MIN]);

    // Stencil control
    cfg.stencil_control.raw         = regs[Reg::mmDB_STENCIL_CONTROL];
    cfg.stencil_refmask_front.raw   = regs[Reg::mmDB_STENCILREFMASK];
    cfg.stencil_refmask_back.raw    = regs[Reg::mmDB_STENCILREFMASK_BF];

    // Depth clamp
    cfg.enable_depth_clamp = (regs[Reg::mmDB_RENDER_OVERRIDE] >> 16) != 1;   // DISABLE_VIEWPORT_CLAMP

    // Viewport
    cfg.viewport_control.raw = regs[Reg::mmPA_CL_VTE_CNTL];
    cfg.z_offset = reinterpret_cast<const float&>(regs[Reg::mmPA_CL_VPORT_ZOFFSET]);
    cfg.z_scale  = reinterpret_cast<const float&>(regs[Reg::mmPA_CL_VPORT_ZSCALE]);

    // Clip space
    cfg.dx_clip_space_enable = (regs[Reg::mmPA_CL_CLIP_CNTL] >> 19) & 1;

    // Calculate final pipeline hash
    state = XXH3_createState();
    XXH3_64bits_reset(state);

    XXH3_64bits_update(state, &cfg.has_vs, sizeof(cfg.has_vs));
    XXH3_64bits_update(state, &cfg.has_ps, sizeof(cfg.has_ps));
    if (cfg.has_vs) XXH3_64bits_update(state, &cfg.vertex_hash, sizeof(cfg.vertex_hash));
    if (cfg.has_ps) XXH3_64bits_update(state, &cfg.pixel_hash, sizeof(cfg.pixel_hash));
    XXH3_64bits_update(state, &cfg.prim_type, sizeof(cfg.prim_type));
    XXH3_64bits_update(state, &cfg.blend_control, sizeof(BlendControl) * 8);
    XXH3_64bits_update(state, &cfg.degamma_enable, sizeof(cfg.degamma_enable));
    XXH3_64bits_update(state, &cfg.depth_control, sizeof(cfg.depth_control));
    XXH3_64bits_update(state, &cfg.depth_clear_enable, sizeof(cfg.depth_clear_enable));
    if (cfg.depth_control.depth_bounds_enable) {
        XXH3_64bits_update(state, &cfg.max_depth_bounds, sizeof(cfg.max_depth_bounds));
        XXH3_64bits_update(state, &cfg.min_depth_bounds, sizeof(cfg.min_depth_bounds));
    }
    if (cfg.depth_control.depth_enable)   XXH3_64bits_update(state, &cfg.enable_depth_clamp, sizeof(cfg.enable_depth_clamp));
    if (cfg.depth_control.stencil_enable) {
        XXH3_64bits_update(state, &cfg.stencil_control, sizeof(cfg.stencil_control));
        XXH3_64bits_update(state, &cfg.stencil_refmask_front, sizeof(cfg.stencil_refmask_front));
        XXH3_64bits_update(state, &cfg.stencil_refmask_back,  sizeof(cfg.stencil_refmask_back));
    }
    XXH3_64bits_update(state, &cfg.viewport_control, sizeof(cfg.viewport_control));
    if (cfg.viewport_control.z_offset_enable) XXH3_64bits_update(state, &cfg.z_offset, sizeof(cfg.z_offset));
    if (cfg.viewport_control.z_scale_enable)  XXH3_64bits_update(state, &cfg.z_scale,  sizeof(cfg.z_scale));
    XXH3_64bits_update(state, &cfg.dx_clip_space_enable, sizeof(cfg.dx_clip_space_enable));
    XXH3_64bits_update(state, &cfg.binding_hash, sizeof(cfg.binding_hash));

    const u64 pipeline_hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    
    if (pipelines.contains(pipeline_hash))
        return *pipelines[pipeline_hash];

    log("Compiling new pipeline\n");
    auto* pipeline = new Pipeline(vert_shader, pixel_shader, fetch_shader, cfg);
    pipelines[pipeline_hash] = pipeline;
    return *pipeline;
}

}   // End namespace PS4::GCN::Vulkan::PipelineCache