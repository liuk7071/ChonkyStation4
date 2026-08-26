#include "ShaderCache.hpp"
#include <Logger.hpp>
#include <Profiler.hpp>
#include <GCN/FetchShader.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/GLSLCompiler.hpp>
#include <xxhash.h>
#include <unordered_map>
#include <mutex>
#include <semaphore>


namespace PS4::GCN::Vulkan::ShaderCache {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

std::unordered_map<u64, CachedShader*> shaders;
std::function<void()> cache_miss_callback = nullptr;

u64 getShaderHash(const u8* code, Shader::ShaderStage stage, ComputeJob* compute_job) {
    // Find shader header
    u32* ptr = (u32*)code;
    while (*ptr != 0x5362724F) {    // "OrbS"
        ptr++;
    }

    // Get the shader hash from the header
    u64 hash;
    ptr += 4;
    std::memcpy(&hash, ptr, sizeof(u64));

    // Hash compute job info if this is a compute shader
    if (stage == Shader::ShaderStage::Compute) {
        XXH3_state_t* state = XXH3_createState();
        XXH3_64bits_reset(state);

        XXH3_64bits_update(state, &hash, sizeof(hash));
        XXH3_64bits_update(state, &compute_job->n_threads_x, sizeof(compute_job->n_threads_x));
        XXH3_64bits_update(state, &compute_job->n_threads_y, sizeof(compute_job->n_threads_y));
        XXH3_64bits_update(state, &compute_job->n_threads_z, sizeof(compute_job->n_threads_z));
        XXH3_64bits_update(state, &compute_job->lds_size_dwords, sizeof(compute_job->lds_size_dwords));

        hash = XXH3_64bits_digest(state);
        XXH3_freeState(state);
    }

    return hash;
}

EShLanguage getEShLanguage(Shader::ShaderStage stage) {
    switch (stage) {
    case Shader::ShaderStage::Vertex:       return EShLangVertex;
    case Shader::ShaderStage::Fragment:     return EShLangFragment;
    case Shader::ShaderStage::Compute:      return EShLangCompute;
    default: Helpers::panic("shader_stage: unreachable");
    }
}

CachedShader* getShader(const u8* code, Shader::ShaderStage stage, FetchShader* fetch_shader, ComputeJob* compute_job) {
    const auto hash = getShaderHash(code, stage, compute_job);

    // Check if this shader was cached, otherwise compile and cache it
    auto it = shaders.find(hash);
    if (it != shaders.end()) {
        auto* cached_shader = it->second;
        if (cached_shader->is_async_compile && !cached_shader->is_compiled) {
            //Profiler::Scope profiler("Time waited on shader compile");
            std::unique_lock<std::mutex> lk(cached_shader->compile_mtx);
            cached_shader->compile_cv.wait(lk, [&]() -> bool { return cached_shader->is_compiled; });
        }
        return cached_shader;
    }

    // Compile it
    //Profiler::Scope profiler("Time waited on shader compile");
    log("Compiling new shader %016llx\n", hash);
    CachedShader* cached_shader = new CachedShader();
    cached_shader->data.hash = hash;
    Shader::decompileShader((u32*)code, stage, cached_shader->data, fetch_shader, compute_job);
    cached_shader->vk_shader = createShaderModule(GCN::compileGLSL(cached_shader->data.source, getEShLanguage(stage), std::format("{:x}.glsl", hash)));
    cached_shader->is_async_compile = false;
    cached_shader->is_compiled = true;

    // Cache it
    shaders[hash] = cached_shader;

    // Call cache miss callback
    if (cache_miss_callback) cache_miss_callback();
    return cached_shader;
}

struct DecompilerTask {
    CachedShader* cached_shader = nullptr;
    u8* code = nullptr;
    Shader::ShaderStage stage;
    std::shared_ptr<FetchShader> fetch_shader = nullptr;
    std::shared_ptr<ComputeJob> compute_job;
    std::shared_ptr<u32[]> regs;
};

std::counting_semaphore<1000000> decompiler_sem { 0 };
std::mutex decompiler_mtx;
std::deque<DecompilerTask> decompiler_tasks;

void decompileWorker() {
    DecompilerTask task;

    while (true) {
        // Wait until we have work to do
        decompiler_sem.acquire();

        // Fetch task
        {
            std::unique_lock<std::mutex> lk(decompiler_mtx);
            task = decompiler_tasks.front();
            decompiler_tasks.pop_front();
        }

        Shader::decompileShader((u32*)task.code, task.stage, task.cached_shader->data, task.fetch_shader.get(), task.compute_job.get(), task.regs.get());
        task.cached_shader->vk_shader = createShaderModule(GCN::compileGLSL(task.cached_shader->data.source, getEShLanguage(task.stage), std::format("{:x}.glsl", task.cached_shader->data.hash)));
        task.cached_shader->is_compiled = true;
        task.cached_shader->compile_cv.notify_all();
    }
}

void initWorkerThreads(int n_threads) {
    for (int i = 0; i < n_threads; i++) {
        auto thread = std::thread(decompileWorker);
        thread.detach();
    }
}

void decompileLater(const u8* code, Shader::ShaderStage stage, std::shared_ptr<FetchShader> fetch_shader, u32* regs, std::shared_ptr<ComputeJob> compute_job) {
    const auto hash = getShaderHash(code, stage, compute_job.get());

    // Check if this shader was already compiled or planned to compile
    if (shaders.contains(hash)) return;

    CachedShader* cached_shader = new CachedShader();
    cached_shader->data.hash        = hash;
    cached_shader->is_async_compile = true;
    cached_shader->is_compiled      = false;
    shaders[hash] = cached_shader;

    DecompilerTask task;
    task.cached_shader  = cached_shader;
    task.code           = (u8*)code;
    task.stage          = stage;
    task.fetch_shader   = fetch_shader;
    task.compute_job    = compute_job;
    task.regs           = std::make_shared<u32[]>(0xd000);
    std::memcpy(task.regs.get(), regs, 0xd000 * sizeof(u32));

    {
        std::unique_lock<std::mutex> lk(decompiler_mtx);
        decompiler_tasks.push_back(std::move(task));
    }
    decompiler_sem.release();
}

void setCallbackForCacheMiss(std::function<void()> func) {
    cache_miss_callback = func;
}

// TODO: Doesn't work with compute because we don't hash the job info (see above)
bool isCached(const u8* code) {
    // Find shader header
    u32* ptr = (u32*)code;
    while (*ptr != 0x5362724F) {    // "OrbS"
        ptr++;
    }

    // Get the shader hash from the header
    u64 hash;
    ptr += 4;
    std::memcpy(&hash, ptr, sizeof(u64));
    return shaders.contains(hash);
}

}   // End namespace PS4::GCN::Vulkan::ShaderCache