#pragma once

#include <Common.hpp>
#include <BitField.hpp>
#include <GCN/Shader/ShaderDecompiler.hpp>
#include <GCN/ComputeJob.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>


namespace PS4::GCN::Vulkan::ShaderCache {

struct CachedShader {
    vk::raii::ShaderModule vk_shader = nullptr;
    Shader::ShaderData data;
    u32 compute_thread_size_x = 0;
    u32 compute_thread_size_y = 0;
    u32 compute_thread_size_z = 0;

    std::atomic<bool> is_async_compile = false;
    std::atomic<bool> is_compiled = false;

    std::mutex compile_mtx;
    std::condition_variable compile_cv;
};

CachedShader* getShader(const u8* code, Shader::ShaderStage stage, FetchShader* fetch_shader, ComputeJob* compute_job = nullptr);
void initWorkerThreads(int n_threads);
void decompileLater(const u8* code, Shader::ShaderStage stage, std::shared_ptr<FetchShader> fetch_shader, u32* regs, std::shared_ptr<ComputeJob> compute_job = nullptr);
void setCallbackForCacheMiss(std::function<void()> func);
bool isCached(const u8* code);

}   // End namespace PS4::GCN::Vulkan::ShaderCache