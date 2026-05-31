#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/ComputeJob.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace PS4::GCN::Vulkan::ShaderCache {

struct CachedShader {
    vk::raii::ShaderModule vk_shader = nullptr;
    Shader::ShaderData data;
    u32 compute_thread_size_x = 0;
    u32 compute_thread_size_y = 0;
    u32 compute_thread_size_z = 0;
};

CachedShader* getShader(const u8* code, Shader::ShaderStage stage, FetchShader* fetch_shader, ComputeJob* compute_job = nullptr);

}   // End namespace PS4::GCN::Vulkan::ShaderCache