#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <vulkan/vulkan_raii.hpp>


namespace PS4::GCN::Vulkan::ShaderCache {

struct CachedShader {
    vk::raii::ShaderModule vk_shader = nullptr;
    Shader::ShaderData data;
};

CachedShader* getShader(const u8* code, Shader::ShaderStage stage, FetchShader* fetch_shader);

}   // End namespace PS4::GCN::Vulkan::ShaderCache