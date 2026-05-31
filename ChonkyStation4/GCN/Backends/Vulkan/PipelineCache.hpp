#pragma once

#include <Common.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/Backends/Vulkan/Pipeline.hpp>
#include <GCN/Backends/Vulkan/ComputePipeline.hpp>
#include <GCN/ComputeJob.hpp>


namespace PS4::GCN::Vulkan::PipelineCache {

Pipeline& getPipeline(const u8* vert_shader_code, const u8* pixel_shader_code, const u8* fetch_shader_code, const u32* regs);
ComputePipeline& getComputePipeline(const ComputeJob& job);

}   // End namespace PS4::GCN::Vulkan::PipelineCache