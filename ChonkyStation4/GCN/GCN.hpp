#pragma once

#include <Common.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <GCN/Backends/Vulkan/VulkanRenderer.hpp>


namespace PS4::GCN {

inline std::unique_ptr<Renderer> renderer;

inline void initVulkan() {
    renderer = std::make_unique<VulkanRenderer>();
    renderer->init();
}

}   // End namespace PS4::GCN