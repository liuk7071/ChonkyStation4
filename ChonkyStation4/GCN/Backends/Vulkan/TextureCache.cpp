#define NOMINMAX
#include "TextureCache.hpp"
#include <Logger.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/Backends/Vulkan/BufferCache.hpp>
#include <GCN/TSharp.hpp>
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

struct TrackedTexture {
    TSharp  tsharp;
    void*   base        = nullptr;
    size_t  size        = 0;
    u32     width       = 0;
    u32     height      = 0;
    u64     page        = 0;
    u64     page_end    = 0;
    bool    dirty       = false;
    vk::raii::Image image       = nullptr;
    vk::raii::DeviceMemory mem  = nullptr;
    vk::raii::ImageView view    = nullptr;
    vk::raii::Sampler sampler   = nullptr;
    vk::DescriptorImageInfo image_info;
};

std::unordered_map<void*, TrackedTexture*> tracked_textures;
std::vector<TrackedTexture*> currently_tracking;

void getVulkanImageInfoForTSharp(TSharp* tsharp, vk::DescriptorImageInfo** out_info) {
    const u32 width = tsharp->width + 1;
    const u32 height = tsharp->height + 1;
    const u32 pitch = tsharp->pitch + 1;

    auto [vk_fmt, pixel_size] = getBufFormatAndSize(tsharp->data_format, tsharp->num_format);
    void* ptr = (void*)(tsharp->base_address << 8);

    size_t img_size;
    if (vk_fmt == vk::Format::eBc3UnormBlock || vk_fmt == vk::Format::eBc1RgbaUnormBlock) {
        const auto blk_width  = (width  + 3) / 4;
        const auto blk_height = (height + 3) / 4;
        img_size = blk_width * blk_height * 16;

    } else img_size = pitch * height * pixel_size;
    
    const uptr   aligned_base = Helpers::alignDown<uptr>((uptr)ptr, Cache::page_size);
    const uptr   aligned_end = Helpers::alignUp<uptr>((uptr)ptr + img_size, Cache::page_size);
    const size_t aligned_size = aligned_end - aligned_base;
    const u64    size_in_pages = aligned_size >> Cache::page_bits;
    const u64    page = aligned_base >> Cache::page_bits;
    const u64    page_end = page + size_in_pages;

    auto reupload_tex = [&](TrackedTexture* tex, bool is_first_upload) {
        auto& img = tex->image;
        device.waitIdle();

        // Check if we need to recreate the image
        // TODO: Just hash the T#?
        if (tex->width != width || tex->height != height || tex->tsharp.data_format != tsharp->data_format || tex->tsharp.num_format != tsharp->num_format) {
            tex->tsharp = *tsharp;
            tex->width = width;
            tex->height = height;
            tex->page = page;
            tex->page_end = page_end;
            auto& mem = tex->mem;
            vk::ImageCreateInfo img_info = {
                .imageType = vk::ImageType::e2D,
                .format = vk_fmt,
                .extent = { width, height, 1 },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = vk::SampleCountFlagBits::e1,
                .tiling = vk::ImageTiling::eOptimal,
                .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                .sharingMode = vk::SharingMode::eExclusive
            };
            img = vk::raii::Image(device, img_info);
            auto mem_requirements = img.getMemoryRequirements();
            vk::MemoryAllocateInfo alloc_info = { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
            mem = vk::raii::DeviceMemory(device, alloc_info);
            img.bindMemory(*mem, 0);
            is_first_upload = true;
        }

        // Transition image layout
        if (is_first_upload)
            transitionImageLayout(img, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        else
            transitionImageLayout(img, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal);

        // Detile the texture
        auto detiled_buf = std::make_unique<u8[]>(img_size);
        const GpaTextureInfo tex_info = gnmTexBuildInfo((GnmTexture*)tsharp);
        GpaError err = gpaTileTextureAll(ptr, img_size, detiled_buf.get(), img_size, &tex_info, GNM_TM_DISPLAY_LINEAR_GENERAL);

        // Upload to a buffer
        const vk::BufferCreateInfo buf_create_info = {
            .size = img_size,
            .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer
                     | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst
                     | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
            .sharingMode = vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VkBuffer raw_buf;
        VmaAllocation alloc;
        VmaAllocationInfo info;
        vmaCreateBuffer(allocator, &*buf_create_info, &alloc_create_info, &raw_buf, &alloc, &info);
        std::memcpy(info.pMappedData, detiled_buf.get(), img_size);

        // Copy buffer to image
        vk::raii::CommandBuffer tmp_cmd = beginCommands();
        const auto buffer_row_length = pitch > width ? pitch : 0;
        if (pitch < width) printf("pitch < width\n");
        vk::BufferImageCopy region = { .bufferOffset = 0, .bufferRowLength = buffer_row_length, .bufferImageHeight = 0, .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, .imageOffset = { 0, 0, 0 }, .imageExtent = { width, height, 1 } };
        tmp_cmd.copyBufferToImage(raw_buf, *img, vk::ImageLayout::eTransferDstOptimal, { region });
        endCommands(tmp_cmd);

        // Free buffer
        vmaDestroyBuffer(allocator, raw_buf, alloc);

        transitionImageLayout(img, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

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
            .viewType = vk::ImageViewType::e2D,
            .format = vk_fmt,
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },

            // TODO: You can in theory change the T# swizzling without changing the texture itself.
            .components = {
                swizzle_map[tsharp->dst_sel_x],
                swizzle_map[tsharp->dst_sel_y],
                swizzle_map[tsharp->dst_sel_z],
                swizzle_map[tsharp->dst_sel_w],
            }
        };
        img_view = vk::raii::ImageView(device, view_info);

        // Create image sampler
        auto& sampler = tex->sampler;
        vk::PhysicalDeviceProperties properties = physical_device.getProperties();
        vk::SamplerCreateInfo sampler_info = {
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
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
    };

    auto invalidate = [&](uptr addr) {
        for (auto it = currently_tracking.begin(); it != currently_tracking.end(); ) {
            auto& tracked_tex = *it;
            if (Helpers::inRangeSized<uptr>(addr, (uptr)tracked_tex->base, tracked_tex->size)) {
                tracked_tex->dirty = true;
                it = currently_tracking.erase(it);
            }
            else it++;
        }
    };

    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    // Check if we are already tracking this texture
    if (tracked_textures.contains(ptr)) {
        auto* tex = tracked_textures[ptr];

        // If the texture was modified, reupload it
        if (tex->dirty) {
            // If the page this texture was in was just dirtied, dirty all textures that are part of this page.
            tex->dirty = false;
            reupload_tex(tex, false);
            for (uptr curr_page = aligned_base; curr_page < aligned_base + aligned_size; curr_page += Cache::page_size) {
                if (!Cache::resetDirty((void*)curr_page, Cache::page_size)) {
                    Cache::track((void*)curr_page, Cache::page_size, invalidate);
                }
            }
            currently_tracking.push_back(tex);
        }

        *out_info = &tex->image_info;
        return;
    }

    log("Tracking new texture\n");
    log("texture size: width=%lld, height=%lld\n", (u32)tsharp->width + 1, (u32)tsharp->height + 1);
    log("texture ptr: %p\n", (void*)tsharp->base_address);
    log("texture dfmt: %d\n", (u32)tsharp->data_format);
    log("texture nfmt: %d\n", (u32)tsharp->num_format);
    log("texture pitch: %d\n", (u32)tsharp->pitch + 1);

    //std::ofstream out;
    //out.open(std::format("{}_{}_{}_{}.bin", width, height, pitch, (tsharp->base_address << 8)), std::ios::binary);
    //out.write((char*)(tsharp->base_address << 8), img_size);

    // Create image
    TrackedTexture* tex = new TrackedTexture();
    tex->tsharp = *tsharp;
    tex->base = ptr;
    tex->size = img_size;
    tex->width = width;
    tex->height = height;
    tex->page = page;
    tex->page_end = page_end;
    auto& img = tex->image;
    auto& mem = tex->mem;
    vk::ImageCreateInfo img_info = {
        .imageType = vk::ImageType::e2D,
        .format = vk_fmt,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .sharingMode = vk::SharingMode::eExclusive
    };
    img = vk::raii::Image(device, img_info);
    auto mem_requirements = img.getMemoryRequirements();
    vk::MemoryAllocateInfo alloc_info = { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
    mem = vk::raii::DeviceMemory(device, alloc_info);
    img.bindMemory(*mem, 0);

    for (uptr curr_page = aligned_base; curr_page < aligned_base + aligned_size; curr_page += Cache::page_size) {
        Cache::track((void*)curr_page, Cache::page_size, invalidate);
    }

    reupload_tex(tex, true);
    tracked_textures[ptr] = tex;
    currently_tracking.push_back(tex);
    *out_info = &tex->image_info;
}

} // End namespace PS4::GCN::Vulkan