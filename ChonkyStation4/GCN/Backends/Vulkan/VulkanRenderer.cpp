#pragma once

#include "VulkanRenderer.hpp"
#include <Logger.hpp>


namespace PS4::GCN {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

void VulkanRenderer::init() {

}

void VulkanRenderer::draw() {
    const auto* vs_ptr = getVSPtr();
    const auto* ps_ptr = getPSPtr();
    log("Vertex Shader address : %p\n", vs_ptr);
    log("Pixel Shader address  : %p\n", ps_ptr);

    if (!vs_ptr) return;

    // TODO: For now fetch shader address is hardcoded to user register 0:1.
    // I think the proper way is to get the register from the SWAPPC instruction in the vertex shader...?
    const auto* fetch_shader_ptr = (u8*)((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_0] | ((u64)regs[Reg::mmSPI_SHADER_USER_DATA_VS_1] << 32));
    log("Fetch Shader address : %p\n", fetch_shader_ptr);

    FetchShader fetch_shader = FetchShader(fetch_shader_ptr);
}

}   // End namespace PS4::GCN