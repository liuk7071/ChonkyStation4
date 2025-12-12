#pragma once

#include <Common.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/Backends/Vulkan/Pipeline.hpp>


namespace PS4::GCN::Vulkan::PipelineCache {

Pipeline& getPipeline(const u8* vert_shader_code, const u8* pixel_shader_code, const u8* fetch_shader_code);

}   // End namespace PS4::GCN::Vulkan::PipelineCache