#define NOMINMAX
#include "TextureCache.hpp"
#include <Logger.hpp>
#include <Profiler.hpp>
#include <GCN/GCN.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/BufferCache.hpp>
#include <GCN/Detiler/gpuaddr.h>
#include <GCN/Detiler/gnm/texture.h>
#include <xxhash.h>
#include <unordered_map>
#include <mutex>
#ifdef _WIN32
#include <Windows.h>
#endif


namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

#ifdef _WIN32
size_t page_size = 0;
#endif

std::mutex cache_mtx;

std::unordered_map<void*, std::vector<TrackedTexture*>> tracked_textures;
std::vector<TrackedTexture*> currently_tracking;

void TrackedTexture::transition(vk::ImageLayout new_layout) {
    if (curr_layout == new_layout) return;
    transitionImageLayout(image, vk_fmt, curr_layout, new_layout);
    curr_layout = new_layout;
}

void getVulkanImageInfoForTSharp(TSharp* tsharp, TrackedTexture** out_info, bool dont_match_num_format, bool is_depth_buffer, vk::Format depth_vk_fmt, bool dont_track_cpu_writes) {
    const bool is_3d = tsharp->type == 10;  // COLOR 3D
    const u32 width = tsharp->width + 1;
    const u32 height = tsharp->height + 1;
    const u32 depth = is_3d ? std::max(tsharp->depth + 1, 1) : 1;
    const u32 pitch = tsharp->pitch + 1;
    //if (tsharp->pow2pad) pitch = std::bit_ceil(pitch);

    void* ptr = (void*)(tsharp->base_address << 8);
    auto [vk_fmt, pixel_size] = getBufFormatAndSize(tsharp->data_format, tsharp->num_format);
    vk_fmt = is_depth_buffer ? depth_vk_fmt : vk_fmt;

    size_t img_size;
    switch (vk_fmt) {
    case vk::Format::eBc4UnormBlock:
    case vk::Format::eBc4SnormBlock:
    case vk::Format::eBc1RgbaUnormBlock:
    case vk::Format::eBc1RgbaSrgbBlock: {
        const auto blk_width  = (width + 3)  / 4;
        const auto blk_height = (height + 3) / 4;
        img_size = blk_width * blk_height * 8;
        break;
    }
    
    case vk::Format::eBc7UnormBlock:
    case vk::Format::eBc7SrgbBlock:
    case vk::Format::eBc6HUfloatBlock:
    case vk::Format::eBc5UnormBlock:
    case vk::Format::eBc5SnormBlock:
    case vk::Format::eBc3UnormBlock:
    case vk::Format::eBc3SrgbBlock:
    case vk::Format::eBc2UnormBlock:
    case vk::Format::eBc2SrgbBlock: {
        const auto blk_width  = (width + 3)  / 4;
        const auto blk_height = (height + 3) / 4;
        img_size = blk_width * blk_height * 16;
        break;
    }

    default: img_size = pitch * height * pixel_size * depth; break;
    }

    // TODO: Tiled 3D textures

    const uptr   aligned_base = Helpers::alignDown<uptr>((uptr)ptr, Cache::page_size);
    const uptr   aligned_end = Helpers::alignUp<uptr>((uptr)ptr + img_size, Cache::page_size);
    const size_t aligned_size = aligned_end - aligned_base;
    const u64    size_in_pages = aligned_size >> Cache::page_bits;
    const u64    page = aligned_base >> Cache::page_bits;
    const u64    page_end = page + size_in_pages;

    auto reupload_tex = [&](TrackedTexture* tex) {
        auto& img = tex->image;

        //tex->invalidate_cnt++;
        //if (tex->invalidate_cnt > 20) {
        //    if (tex->invalidate_cnt % 20 != 0)
        //        return;
        //}

        // Transition image layout
        endRendering();
        tex->transition(vk::ImageLayout::eTransferDstOptimal);

        // Detile the texture
        void* img_ptr = ptr;
        auto detiled_buf = std::make_unique<u8[]>(img_size);

        if (tex->tsharp.tiling_index != GNM_TM_DISPLAY_LINEAR_GENERAL) {
            //Profiler::add("Detiled textures", 1);
            //Profiler::Scope profiler("Detiler time");
            const GpaTextureInfo tex_info = gnmTexBuildInfo((GnmTexture*)tsharp);
            GpaError err = gpaTileTextureAll(ptr, img_size, detiled_buf.get(), img_size, &tex_info, GNM_TM_DISPLAY_LINEAR_GENERAL);
            img_ptr = detiled_buf.get();
        }

        // Upload to a buffer
        auto [buf, buf_ptr] = Cache::getMappedBufferForFrame(img_size);
        std::memcpy(buf_ptr, img_ptr, img_size);

        // Copy buffer to image
        const auto buffer_row_length = pitch >= width ? pitch : 0;
        if (pitch < width) printf("pitch < width\n");
        vk::BufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = buffer_row_length,
            .bufferImageHeight = height,
            .imageSubresource = {
                !tex->is_depth_buffer ? vk::ImageAspectFlagBits::eColor : vk::ImageAspectFlagBits::eDepth,
                0, 0, 1
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { width, height, depth }
        };
        cmd_bufs[frame_idx].copyBufferToImage(buf, *img, vk::ImageLayout::eTransferDstOptimal, { region });
    };

    auto invalidate = [&](uptr addr) {
        for (auto it = currently_tracking.begin(); it != currently_tracking.end(); ) {
            auto& tracked_tex = *it;
            if (!tracked_tex) {
                it = currently_tracking.erase(it);
                continue;
            }

            if (Helpers::inRangeSized<uptr>(addr, (uptr)tracked_tex->base, tracked_tex->size)) {
                tracked_tex->dirty = true;

                // Unprotect the other pages to reduce the number of page faults.
                //const uptr   base = Helpers::alignDown<uptr>((uptr)tracked_tex->base, Cache::page_size);
                //const uptr   end = Helpers::alignUp<uptr>((uptr)tracked_tex->base + tracked_tex->size, Cache::page_size);
                //for (u64 page = base >> Cache::page_bits; page < (end >> Cache::page_bits); page++)
                //    Cache::unprotect(page);

                it = currently_tracking.erase(it);
            }
            else it++;
        }
    };

    //auto lk = std::unique_lock<std::mutex>(cache_mtx);

    // Check if we are already tracking this texture
    if (tracked_textures.contains(ptr)) {
        // Find one that matches the size and format
        for (auto& tracked_tex : tracked_textures[ptr]) {
            if (   tracked_tex->width  == width
                && tracked_tex->height == height
                && (tracked_tex->depth == depth || !is_3d)
                && tracked_tex->tsharp.data_format == tsharp->data_format
                && (tracked_tex->tsharp.num_format == tsharp->num_format || dont_match_num_format)
               ) {
                auto* tex = tracked_tex;
                if (is_depth_buffer && !tex->is_depth_buffer) {
                    //Profiler::add("Dead textures", 1);
                    tex->dead = true;
                    continue;
                }

                if (tex->dead) continue;

                // If the texture was modified, reupload it
                if (tex->dirty) {
                    tex->tsharp = *tsharp;
                    // If the page this texture was in was just dirtied, dirty all textures that are part of this page.
                    tex->dirty = false;
                    reupload_tex(tex);
                    for (uptr curr_page = aligned_base; curr_page < aligned_base + aligned_size; curr_page += Cache::page_size) {
                        if (!dont_track_cpu_writes && !Cache::resetDirty((void*)curr_page, Cache::page_size)) {
                            Cache::track((void*)curr_page, Cache::page_size, invalidate);
                        }
                    }
                    currently_tracking.push_back(tex);
                }

                tex->last_used_frame = GCN::global_flip_counter;
                *out_info = tex;
                return;
            }
        }
    }

    log("Tracking new texture\n");
    log("texture size: width=%lld, height=%lld, depth=%lld\n", (u32)tsharp->width + 1, (u32)tsharp->height + 1, (u32)tsharp->depth + 1);
    log("texture ptr: %p\n", ptr);
    log("texture dfmt: %d\n", (u32)tsharp->data_format);
    log("texture nfmt: %d\n", (u32)tsharp->num_format);
    log("texture pitch: %d\n", (u32)tsharp->pitch + 1);

    //std::ofstream out;
    //out.open(std::format("{}_{}_{}_{}.bin", width, height, pitch, (tsharp->base_address << 8)), std::ios::binary);
    //out.write((char*)(tsharp->base_address << 8), img_size);

    //Profiler::add("New textures", 1);

    // Create image
    TrackedTexture* tex = new TrackedTexture();
    tex->tsharp = *tsharp;
    tex->base = ptr;
    tex->size = img_size;
    tex->width = width;
    tex->height = height;
    tex->depth = depth;
    tex->page = page;
    tex->page_end = page_end;
    tex->is_depth_buffer = is_depth_buffer;
    tex->vk_fmt = vk_fmt;
    auto& img = tex->image;
    auto& mem = tex->mem;

    const bool is_compressed = [&]() -> bool {
        switch ((DataFormat)tex->tsharp.data_format) {
        case DataFormat::FormatBc1:
        case DataFormat::FormatBc2:
        case DataFormat::FormatBc3:
        case DataFormat::FormatBc4:
        case DataFormat::FormatBc5:
        case DataFormat::FormatBc6:
        case DataFormat::FormatBc7:
            return true;
        default:
            return false;
        }
    }();

    vk::Flags<vk::ImageUsageFlagBits> attachment_bits = {};
    if (!is_compressed)
        attachment_bits = (!is_depth_buffer ? vk::ImageUsageFlagBits::eColorAttachment : vk::ImageUsageFlagBits::eDepthStencilAttachment) | vk::ImageUsageFlagBits::eAttachmentFeedbackLoopEXT;

    vk::ImageCreateInfo img_info = {
        .imageType = !is_3d ? vk::ImageType::e2D : vk::ImageType::e3D,
        .format = vk_fmt,
        .extent = { width, height, depth },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eAttachmentFeedbackLoopEXT | attachment_bits,
        .sharingMode = vk::SharingMode::eExclusive
    };

    if (!is_depth_buffer)
        img_info.usage |= vk::ImageUsageFlagBits::eStorage;

    img = vk::raii::Image(device, img_info);
    auto mem_requirements = img.getMemoryRequirements();
    vk::MemoryAllocateInfo alloc_info = { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
    mem = vk::raii::DeviceMemory(device, alloc_info);
    img.bindMemory(*mem, 0);
    
    // Set debug name
    if (device.getDispatcher()->vkSetDebugUtilsObjectNameEXT)
        device.setDebugUtilsObjectNameEXT(*img, std::format("Texture @ {}", ptr));

    // Create image view
    vk::ComponentSwizzle swizzle_map[] = {
        vk::ComponentSwizzle::eZero,    // 0 - DSEL_0
        vk::ComponentSwizzle::eOne,     // 1 - DSEL_1
        vk::ComponentSwizzle::eR,       // 2 - invalid
        vk::ComponentSwizzle::eG,       // 3 - invalid
        vk::ComponentSwizzle::eR,       // 4 - DSEL_R
        vk::ComponentSwizzle::eG,       // 5 - DSEL_G
        vk::ComponentSwizzle::eB,       // 6 - DSEL_B
        vk::ComponentSwizzle::eA,       // 7 - DSEL_A
    };
    auto& img_view = tex->view;
    vk::ImageViewCreateInfo view_info = {
        .image = *img,
        .viewType = !is_3d ? vk::ImageViewType::e2D : vk::ImageViewType::e3D,
        .format = vk_fmt,
        .subresourceRange = {
            !is_depth_buffer ? vk::ImageAspectFlagBits::eColor : vk::ImageAspectFlagBits::eDepth,
            0, 1,
            0, 1
        },

        // TODO: You can in theory change the T# swizzling without changing the texture itself.
        .components = {
            swizzle_map[tsharp->dst_sel_x],
            swizzle_map[tsharp->dst_sel_y],
            swizzle_map[tsharp->dst_sel_z],
            swizzle_map[tsharp->dst_sel_w],
        },
    };
    img_view = vk::raii::ImageView(device, view_info);

    // Create image sampler
    auto& sampler = tex->sampler;
    vk::SamplerCreateInfo sampler_info = {
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eNearest,

        // Hack for Tomb Raider, fix when I implement samplers
        .addressModeU = width == 256 ? vk::SamplerAddressMode::eClampToEdge : vk::SamplerAddressMode::eRepeat,
        .addressModeV = width == 256 ? vk::SamplerAddressMode::eClampToEdge : vk::SamplerAddressMode::eRepeat,
        .addressModeW = width == 256 ? vk::SamplerAddressMode::eClampToEdge : vk::SamplerAddressMode::eRepeat,

        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::False,
        .maxAnisotropy = 1.0f,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eLessOrEqual,
        .borderColor = vk::BorderColor::eFloatTransparentBlack
    };
    sampler = vk::raii::Sampler(device, sampler_info);

    tex->image_info = {
        .sampler = *sampler,
        .imageView = *img_view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };

    tex->image_info_general = {
        .sampler = *sampler,
        .imageView = *img_view,
        .imageLayout = vk::ImageLayout::eGeneral
    };

    if (!dont_track_cpu_writes) {
        for (uptr curr_page = aligned_base; curr_page < aligned_base + aligned_size; curr_page += Cache::page_size) {
            Cache::track((void*)curr_page, Cache::page_size, invalidate);
        }
    }

    if (!is_depth_buffer)
        reupload_tex(tex);
    
    if (!dont_track_cpu_writes) {
        currently_tracking.push_back(tex);
    }
    
    tracked_textures[ptr].push_back(tex);

    tex->last_used_frame = GCN::global_flip_counter;
    *out_info = tex;
}

void freeUnusedTextures() {
    constexpr u64 treshold = 1000;
    for (auto it = tracked_textures.begin(); it != tracked_textures.end(); ) {
        for (auto tex = it->second.begin(); tex != it->second.end(); ) {
            if ((GCN::global_flip_counter - (*tex)->last_used_frame) > treshold) {
                // Unprotect
                const uptr   base = Helpers::alignDown<uptr>((uptr)(*tex)->base, Cache::page_size);
                const uptr   end = Helpers::alignUp<uptr>((uptr)(*tex)->base + (*tex)->size, Cache::page_size);
                for (u64 page = base >> Cache::page_bits; page < (end >> Cache::page_bits); page++)
                    Cache::unprotect(page);

                //Profiler::add("Released textures", 1);
                delete *tex;
                tex = it->second.erase(tex);
            }
            else
                tex++;
        }

        if (it->second.empty())
            it = tracked_textures.erase(it);
        else
            it++;
    }
}

} // End namespace PS4::GCN::Vulkan