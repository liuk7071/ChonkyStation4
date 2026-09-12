#include "NsightAftermath.hpp"
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#include <chrono>
#include <thread>


namespace PS4::GCN::Vulkan::NVIDIA {

static constexpr bool force_disable = false;

void crashDumpCallback(const void* crash_dump, const u32 crash_dump_size, void* user_data) {
    std::ofstream file("crash.nv-gpudmp", std::ios::binary);

    file.write((const char*)crash_dump, crash_dump_size);
}

void shaderDebugInfoCallback(const void* shader_debug_info, const u32 shader_debug_info_size, void* user_data) {
    const fs::path path = "./NVIDIA/shader_debug_information";

    GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier;
    GFSDK_Aftermath_GetShaderDebugInfoIdentifier(GFSDK_Aftermath_Version_API, shader_debug_info, shader_debug_info_size, &identifier);

    std::filesystem::create_directories(path);
    const auto filename = std::format("{}/{:x}{:x}.nvdbg", path.generic_string(), identifier.id[0], identifier.id[1]);
    std::ofstream file(filename.c_str(), std::ios::binary);
    file.write((const char*)shader_debug_info, shader_debug_info_size);
    file.flush();
    file.close();
}

void crashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription add_description, void* user_data) {

}

void resolveMarkerCallback(const void*, const u32, void*, PFN_GFSDK_Aftermath_ResolveMarker resolve_marker) {

}

void initAftermath() {
    if (force_disable) return;

    const auto result = GFSDK_Aftermath_EnableGpuCrashDumps(
        GFSDK_Aftermath_Version_API,
        GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
        GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks,
        crashDumpCallback,
        shaderDebugInfoCallback,
        crashDumpDescriptionCallback,
        resolveMarkerCallback,
        nullptr
    );

    if (result != GFSDK_Aftermath_Result_Success) {
        Helpers::panic("GFSDK_Aftermath_EnableGpuCrashDumps failed\n");
    }

    printf("Initialized NVIDIA Nsight Aftermath\n");
}

bool isAftermathEnabled() {
    return !force_disable;
}

void waitForCrashDump() {
    if (force_disable) return;

    // https://github.com/NVIDIA/nsight-aftermath-samples/blob/master/VkHelloNsightAftermath/VkHelloNsightAftermath.cpp:
    // Device lost notification is asynchronous to the NVIDIA display
    // driver's GPU crash handling. Give the Nsight Aftermath GPU crash dump
    // thread some time to do its work before terminating the process.

    printf("Generating GPU crash dump...\n");

    const auto timeout = std::chrono::seconds(5);
    const auto start = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::milliseconds::zero();

    GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
    GFSDK_Aftermath_GetCrashDumpStatus(&status);
    while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed && status != GFSDK_Aftermath_CrashDump_Status_Finished && elapsed < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        GFSDK_Aftermath_GetCrashDumpStatus(&status);
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    }

    if (status == GFSDK_Aftermath_CrashDump_Status_Finished)
        printf("GPU crash dump saved to crash.nv-gpudmp\n");
    else
        printf("Failed to generate GPU crash dump\n");
}

void endAftermath() {
    GFSDK_Aftermath_DisableGpuCrashDumps();
}

}   // End namespace PS4::GCN::Vulkan::NVIDIA