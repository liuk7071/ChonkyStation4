#include "RenderTarget.hpp"
#include <GCN/GCN.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/TextureCache.hpp>
#include <GCN/VSharp.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Backends/Renderer.hpp>
#include <unordered_map>


namespace PS4::GCN::Vulkan::RenderTarget {

// Hack: keep track of the color targets we used within a frame to set LoadOp to clear the first time they're used
std::unordered_map<TrackedTexture*, bool> used_bufs;

Attachment getVulkanAttachmentForColorTarget(ColorTarget* rt, bool* save) {
    Attachment attachment;

    // Build a texture descriptor with the color target info
    // TODO: It's best to adapt our texture cache to be a more generic "Vulkan image cache" in the future
    TSharp tsharp;
    const u32 pitch_tile_max = rt->pitch_tile_max & 0x7ff;
    const auto pitch = (pitch_tile_max + 1) * 8;
    //const u32 slice_tile_max = rt->slice_tile_max & 0x3fffff;
    //tsharp.height = ((slice_tile_max + 1) * 64) / tsharp.width;
    

    tsharp.width  = rt->width - 1;
    tsharp.height = rt->height - 1;
    tsharp.pitch  = pitch - 1;
    tsharp.base_address = (uptr)rt->base >> 8;
    tsharp.data_format  = rt->info.format; 
    tsharp.num_format   = rt->info.number_type;
    tsharp.dst_sel_x = DSEL_R;
    tsharp.dst_sel_y = DSEL_G;
    tsharp.dst_sel_z = DSEL_B;
    tsharp.dst_sel_w = DSEL_A;
    tsharp.tiling_index = 31;

    //printf("rt: %p, %dx%d fmt %d,%d\n", tsharp.base_address << 8, renderer->color_rt_dim[rt->idx].width, renderer->color_rt_dim[rt->idx].height, rt->info.format.Value(), rt->info.number_type.Value());

    // Get an image from our cache
    TrackedTexture* out_info;
    endRendering();
    getVulkanImageInfoForTSharp(&tsharp, &out_info);
    
    // TODO: This doesn't detect feedback loops that happen in the middle of a renderpass
    const bool was_bound = out_info->was_bound;
    out_info->was_bound = false;
    out_info->transition(!was_bound ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT);
    
    if (was_bound) {
        out_info->image_info.imageLayout = vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT;
    }
    
    attachment.tex = out_info;
    attachment.has_feedback_loop = was_bound;

    // Clear hack
    const auto clear_word0 = renderer->regs[Reg::mmCB_COLOR0_CLEAR_WORD0];
    const auto clear_word1 = renderer->regs[Reg::mmCB_COLOR0_CLEAR_WORD1];
    const float alpha = !clear_word0 && !clear_word1 ? 0.0f : 1.0f;

    *save = true;
    vk::AttachmentLoadOp load_op = vk::AttachmentLoadOp::eLoad;
    if (!used_bufs.contains(out_info)) {
        used_bufs[out_info] = true;
        load_op = vk::AttachmentLoadOp::eClear;
        *save = false;
    }

    attachment.vk_attachment = vk::RenderingAttachmentInfo {
        .imageView = out_info->image_info.imageView,
        .imageLayout = !was_bound ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eAttachmentFeedbackLoopOptimalEXT,
        .loadOp = load_op,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearValue {
            vk::ClearColorValue { std::array<float,4>{0, 0, 0, alpha} }
        }
    };

    return attachment;
}

Attachment getVulkanAttachmentForDepthTarget(DepthTarget* depth, bool* save) {
    auto get_dfmt_nfmt = [&]() -> std::pair<DataFormat, NumberFormat> {
        switch (depth->z_info.format) {
        case 1: return { DataFormat::Format16, NumberFormat::Unorm };
        case 3: return { DataFormat::Format32, NumberFormat::Float };
        default: Helpers::panic("invalid depth format %d\n", depth->z_info.format.Value());
        }
    };
    
    auto get_vk_fmt = [&]() -> vk::Format {
        switch (depth->z_info.format) {
        case 1: return vk::Format::eD16Unorm;
        case 3: return vk::Format::eD32Sfloat;
        default: Helpers::panic("invalid depth format %d\n", depth->z_info.format.Value());
        }
    };

    Attachment attachment;

    auto [dfmt, nfmt] = get_dfmt_nfmt();
    auto vk_fmt       = get_vk_fmt();

    // Build a texture descriptor with the depth target info
    // TODO: It's best to adapt our texture cache to be a more generic "Vulkan image cache" in the future
    TSharp tsharp;
    tsharp.width    = depth->width - 1;
    tsharp.height   = depth->height - 1;
    tsharp.pitch    = tsharp.width;
    tsharp.base_address = (uptr)depth->base >> 8;
    tsharp.data_format  = (u32)dfmt;
    tsharp.num_format   = (u32)nfmt;
    tsharp.dst_sel_x = DSEL_R;
    tsharp.dst_sel_y = DSEL_G;
    tsharp.dst_sel_z = DSEL_B;
    tsharp.dst_sel_w = DSEL_A;
    tsharp.tiling_index = 31;

    // Get an image from our cache
    TrackedTexture* out_info;
    endRendering();
    getVulkanImageInfoForTSharp(&tsharp, &out_info, true, vk_fmt);
    out_info->transition(vk::ImageLayout::eDepthAttachmentOptimal);
    out_info->was_targeted = true;

    attachment.tex = out_info;

    // Clear hack
    *save = true;
    vk::AttachmentLoadOp load_op = vk::AttachmentLoadOp::eLoad;
    if (!used_bufs.contains(out_info)) {
        used_bufs[out_info] = true;
        load_op = vk::AttachmentLoadOp::eClear;
        *save = false;
    }

    attachment.vk_attachment = vk::RenderingAttachmentInfo {
        .imageView = out_info->image_info.imageView,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = load_op,
        //.loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearValue {
            vk::ClearDepthStencilValue { 0.0f, 0 }
        }
    };

    return attachment;
}

void reset() {
    for (auto& [tex, bound] : used_bufs)
        tex->was_targeted = false;
    used_bufs.clear();
}

}   // End namespace PS4::GCN::Vulkan::RenderTarget