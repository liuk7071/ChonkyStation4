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

enum class PrimitiveType : u32 {
    None                = 0,
    PointList           = 1,
    LineList            = 2,
    LineStrip           = 3,
    TriangleList        = 4,
    TriangleFan         = 5,
    TriangleStrip       = 6,
    PatchPrimitive      = 9,
    AdjLineList         = 10,
    AdjLineStrip        = 11,
    AdjTriangleList     = 12,
    AdjTriangleStrip    = 13,
    RectList            = 17,
    LineLoop            = 18,
    QuadList            = 19,
    QuadStrip           = 20,
    Polygon             = 21,
};

enum class BlendFactor : u32 {
    Zero                    = 0,
    One                     = 1,
    SrcColor                = 2,
    OneMinusSrcColor        = 3,
    SrcAlpha                = 4,
    OneMinusSrcAlpha        = 5,
    DstAlpha                = 6,
    OneMinusDstAlpha        = 7,
    DstColor                = 8,
    OneMinusDstColor        = 9,
    SrcAlphaSaturate        = 10,
    ConstantColor           = 13,
    OneMinusConstantColor   = 14,
    Src1Color               = 15,
    InvSrc1Color            = 16,
    Src1Alpha               = 17,
    InvSrc1Alpha            = 18,
    ConstantAlpha           = 19,
    OneMinusConstantAlpha   = 20,
};

enum class BlendFunc : u32 {
    Add             = 0,
    Subtract        = 1,
    Min             = 2,
    Max             = 3,
    ReverseSubtract = 4,
};

enum class CompareFunc : u32 {
    Never           = 0,
    Less            = 1,
    Equal           = 2,
    LessEqual       = 3,
    Greater         = 4,
    NotEqual        = 5,
    GreaterEqual    = 6,
    Always          = 7,
};

enum class StencilOp : u32 {
    Keep            = 0,
    Zero            = 1,
    One             = 2,
    ReplaceTest     = 3,
    ReplaceOpVal    = 4,
    AddOpValClamp   = 5,
    SubOpValClamp   = 6,
    Invert          = 7,
    AddOpValWrap    = 8,
    SubOpValWrap    = 9,
    AndOpVal        = 10,
    OrOpVal         = 11,
    XorOpVal        = 12,
    NandOpVal       = 13,
    NorOpVal        = 14,
    XnorOpVal       = 15,
};

union BlendControl {
    u32 raw = 0;
    BitField<0,  5, u32> src_blend;
    BitField<5,  3, u32> color_func;
    BitField<8,  5, u32> dst_blend;
    BitField<16, 5, u32> alpha_src_blend;
    BitField<21, 3, u32> alpha_func;
    BitField<24, 5, u32> alpha_dst_blend;
    BitField<29, 1, u32> separate_alpha_blend;
    BitField<30, 1, u32> enable;
    BitField<31, 1, u32> disable_rop;
};

union DepthControl {
    u32 raw = 0;
    BitField<0,  1, u32> stencil_enable;
    BitField<1,  1, u32> depth_enable;
    BitField<2,  1, u32> depth_write_enable;
    BitField<3,  1, u32> depth_bounds_enable;
    BitField<4,  3, u32> depth_func;
    BitField<7,  1, u32> stencil_backface_enable;
    BitField<8,  3, u32> stencil_func_front;
    BitField<20, 3, u32> stencil_func_back;
    BitField<30, 1, u32> enable_color_writes_on_depth_fail;
    BitField<31, 1, u32> disable_color_writes_on_depth_fail;
};

union StencilControl {
    u32 raw = 0;
    BitField<0,  4, u32> front_fail_op;
    BitField<4,  4, u32> front_pass_op;
    BitField<8,  4, u32> front_depth_fail_op;
    BitField<12, 4, u32> back_fail_op;
    BitField<16, 4, u32> back_pass_op;
    BitField<20, 4, u32> back_depth_fail_op;
};

union StencilRefMask {
    u32 raw = 0;
    BitField<0,  8, u32> stencil_ref;
    BitField<8,  8, u32> stencil_compare_mask;
    BitField<16, 8, u32> stencil_write_mask;
    BitField<24, 8, u32> stencil_op_val;
};

union ViewportTransformControl {
    u32 raw = 0;
    BitField<0,  1, u32> x_scale_enable;
    BitField<1,  1, u32> x_offset_enable;
    BitField<2,  1, u32> y_scale_enable;
    BitField<3,  1, u32> y_offset_enable;
    BitField<4,  1, u32> z_scale_enable;
    BitField<5,  1, u32> z_offset_enable;
    BitField<8,  1, u32> vtx_xy_fmt;
    BitField<9,  1, u32> vtx_z_fmt;
    BitField<10, 1, u32> vtx_w0_fmt;
};

struct PipelineConfig {
    // Shader
    bool has_vs = false;
    bool has_ps = false;
    u64 vertex_hash = 0;
    u64 pixel_hash = 0;

    // Draw primitive
    u32 prim_type = 0;

    // Blending
    BlendControl blend_control[8];

    // Color
    bool degamma_enable = false;

    // Depth / Stencil
    DepthControl   depth_control;
    bool depth_clear_enable = false;
    StencilControl stencil_control;
    float max_depth_bounds = 0.0f;
    float min_depth_bounds = 0.0f;
    bool enable_depth_clamp = false;
    StencilRefMask stencil_refmask_front;
    StencilRefMask stencil_refmask_back;

    // Viewport
    ViewportTransformControl viewport_control;
    float z_offset = 0.0f;
    float z_scale  = 0.0f;

    // Clip space
    bool dx_clip_space_enable = false;

    // Other hashes
    u64 binding_hash = 0;   // Hash calculated on the fetch shader binding info
};

class Pipeline {
public:
    Pipeline(ShaderCache::CachedShader* vert_shader, ShaderCache::CachedShader* pixel_shader, FetchShader fetch_shader, PipelineConfig& cfg);
    ShaderCache::CachedShader* vert_shader;
    ShaderCache::CachedShader* pixel_shader;
    vk::raii::ShaderModule tess_control_shader = nullptr;
    vk::raii::ShaderModule tess_eval_shader = nullptr;
    FetchShader fetch_shader;
    PipelineConfig cfg;
    float min_viewport_depth = 0.0f;
    float max_viewport_depth = 0.0f;
    bool has_blend_constants = false;

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
        return graphics_pipeline;
    }
    vk::raii::PipelineLayout& getVkPipelineLayout() {
        return pipeline_layout;
    }

    std::vector<VertexBinding>* gatherVertices();
    std::vector<vk::WriteDescriptorSet> uploadBuffersAndTextures(PushConstants** push_constants_ptr, TrackedTexture* rt, bool* has_feedback_loop);
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
    std::deque<vk::DescriptorBufferInfo> buffer_info;
    PushConstants push_constants;
};

}   // End namespace PS4::GCN::Vulkan