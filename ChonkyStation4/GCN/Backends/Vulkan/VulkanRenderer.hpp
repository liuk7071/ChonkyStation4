#pragma once

#include <Common.hpp>
#include <GCN/Backends/Renderer.hpp>


namespace PS4::GCN {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() : Renderer() {}

    void init() override;
    void draw() override;
};

}   // End namespace PS4::GCN