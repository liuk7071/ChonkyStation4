#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/FetchShader.hpp>
#include <deque>
#include "vk_mem_alloc.h"


class VSharp;

namespace PS4::GCN::Vulkan {

enum class PrimitiveType : u32 {
    None = 0,
    PointList = 1,
    LineList = 2,
    LineStrip = 3,
    TriangleList = 4,
    TriangleFan = 5,
    TriangleStrip = 6,
    PatchPrimitive = 9,
    AdjLineList = 10,
    AdjLineStrip = 11,
    AdjTriangleList = 12,
    AdjTriangleStrip = 13,
    RectList = 17,
    LineLoop = 18,
    QuadList = 19,
    QuadStrip = 20,
    Polygon = 21,
};

enum class BlendFactor : u32 {
    Zero = 0,
    One = 1,
    SrcColor = 2,
    OneMinusSrcColor = 3,
    SrcAlpha = 4,
    OneMinusSrcAlpha = 5,
    DstAlpha = 6,
    OneMinusDstAlpha = 7,
    DstColor = 8,
    OneMinusDstColor = 9,
    SrcAlphaSaturate = 10,
    ConstantColor = 13,
    OneMinusConstantColor = 14,
    Src1Color = 15,
    InvSrc1Color = 16,
    Src1Alpha = 17,
    InvSrc1Alpha = 18,
    ConstantAlpha = 19,
    OneMinusConstantAlpha = 20,
};

enum class BlendFunc : u32 {
    Add = 0,
    Subtract = 1,
    Min = 2,
    Max = 3,
    ReverseSubtract = 4,
};

union BlendControl {
    u32 raw = 0;
    BitField<0,  5, u32> src_blend;
    BitField<5,  3, u32> color_func;
    BitField<8,  5, u32> dst_blend;
    BitField<16, 5, u32> alpha_src_blend;
    BitField<21, 3, u32> alpha_func;
    BitField<24, 5, u32> dst_alpha;
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
    BitField<8,  3, u32> stencil_func;
    BitField<20, 3, u32> stencil_func_backface;
    BitField<30, 1, u32> enable_color_writes_on_depth_fail;
    BitField<31, 1, u32> disable_color_writes_on_depth_fail;
};

struct PipelineConfig {
    u32 prim_type = 0;
    BlendControl blend_control[8];
    
    DepthControl depth_control;
    bool enable_depth_clamp = false;

    u64 vertex_hash = 0;
    u64 pixel_hash = 0;
    u64 binding_hash = 0;   // Hash calculated on the fetch shader binding info
};

class Pipeline {
public:
    Pipeline(Shader::ShaderData vert_shader, Shader::ShaderData pixel_shader, FetchShader fetch_shader, PipelineConfig& cfg);
    Shader::ShaderData vert_shader;
    Shader::ShaderData pixel_shader;
    FetchShader fetch_shader;
    PipelineConfig cfg;

    struct VertexBinding {
        FetchShaderVertexBinding fetch_shader_binding;
        vk::Buffer buf = nullptr;
        size_t offs_in_buf = 0;
        VmaAllocation alloc;
    };

    struct PushConstants {
        u16 stride[32];
        u16 fmt[32];
    };

    vk::raii::Pipeline& getVkPipeline() {
        return graphics_pipeline;
    }
    vk::raii::PipelineLayout& getVkPipelineLayout() {
        return pipeline_layout;
    }

    std::vector<VertexBinding>* gatherVertices();
    std::vector<vk::WriteDescriptorSet> uploadBuffersAndTextures(PushConstants** push_constants_ptr);
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

    vk::raii::ShaderModule createShaderModule(const std::vector<u32>& code);
};

}   // End namespace PS4::GCN::Vulkan