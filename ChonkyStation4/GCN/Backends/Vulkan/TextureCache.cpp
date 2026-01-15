#define NOMINMAX
#include "TextureCache.hpp"
#include <Logger.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Detiler/gpuaddr.h>
#include <GCN/Detiler/gnm/texture.h>
#include <xxhash.h>
#include <unordered_map>
#ifdef _WIN32
#include <Windows.h>
#endif


namespace PS4::GCN::Vulkan {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

struct TextureCacheEntry {
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory mem = nullptr;
    vk::raii::ImageView view = nullptr;
    vk::raii::Sampler sampler = nullptr;
    vk::DescriptorImageInfo image_info;
};

std::unordered_map<u64, TextureCacheEntry*> cache;
std::unordered_map<void*, u64> addr_hash_map;

#ifdef _WIN32
size_t page_size = 0;
#endif

struct TrackedTexture {
    TSharp tsharp;
    void* base  = nullptr;
    size_t size = 0;
    u32 width = 0;
    u32 height = 0;
    bool dirty  = false;
    bool false_positive = false;    // Set when something accesses the same memory page as the texture, but not the texture itself
    vk::raii::Image image       = nullptr;
    vk::raii::DeviceMemory mem  = nullptr;
    vk::raii::ImageView view    = nullptr;
    vk::raii::Sampler sampler   = nullptr;
    vk::DescriptorImageInfo image_info;

    void protect() {
#ifdef _WIN32
        const auto aligned_start = Helpers::alignDown<u64>((u64)base, page_size);
        const auto aligned_end   = Helpers::alignUp<size_t>((u64)base + size, page_size);

        DWORD old_protect;
        if (!VirtualProtect((void*)aligned_start, aligned_end - aligned_start, PAGE_READONLY, &old_protect))
            Helpers::panic("TrackedTexture::protect: VirtualProtect failed");
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }

    void unprotect() {
#ifdef _WIN32
        const auto aligned_start = Helpers::alignDown<u64>((u64)base, page_size);
        const auto aligned_end = Helpers::alignUp<size_t>((u64)base + size, page_size);

        DWORD old_protect;
        if (!VirtualProtect((void*)aligned_start, aligned_end - aligned_start, PAGE_READWRITE, &old_protect))
            Helpers::panic("TrackedTexture::unprotect: VirtualProtect failed");
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }
};

std::unordered_map<void*, TrackedTexture*> tracked_textures;

#ifdef _WIN32

LONG CALLBACK exceptionHandler(EXCEPTION_POINTERS* info) {
    const auto* record = info->ExceptionRecord;
    if (record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;

    const bool is_write = record->ExceptionInformation[0] == 1;
    if (!is_write) return EXCEPTION_CONTINUE_SEARCH;

    // Check all tracked textures to see if this address is inside any of them.
    // TODO: Come up with a better solution than this loop
    void* addr = (void*)record->ExceptionInformation[1];
    for (auto& tex : tracked_textures) {
        // Align to page size.
        const auto aligned_start = Helpers::alignDown<u64>((u64)tex.second->base, page_size);
        const auto aligned_end   = Helpers::alignUp<size_t>((u64)tex.second->base + tex.second->size, page_size);

        // This leads to many unnecessary texture reuploads, but I don't know any better way.
        if (Helpers::inRangeSized<u64>((u64)addr, aligned_start, aligned_end - aligned_start)) {
            tex.second->dirty = true;
            
            // TODO: To avoid unnecessary reuploads, we don't reupload the texture if the access was inside the same page as the texture but not within the texture itself.
            // This is a problem, because if after this point the game does edit the texture, we won't catch it.
            // This problem happens in We Are Doomed.
            if (!Helpers::inRangeSized<u64>((u64)addr, (u64)tex.second->base, tex.second->size))
                tex.second->false_positive = true;

            // Restore write access and continue execution
            tex.second->unprotect();
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void initTextureCache() {
    // Setup exception handler
#ifdef _WIN32
    if (!AddVectoredExceptionHandler(1, exceptionHandler))
        Helpers::panic("initTextureCache: failed to register exception handler");

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
#else
    Helpers::panic("Unsupported platform\n");
#endif
}

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
    
    auto reupload_tex = [&](TrackedTexture* tex, bool is_first_upload) {
        if (tex->false_positive) {
            tex->false_positive = false;
            
            if (tex->width == width && tex->height == height && tex->tsharp.data_format == tsharp->data_format && tex->tsharp.num_format == tsharp->num_format)
                return;
        }

        auto& img = tex->image;
        device.waitIdle();

        // Check if we need to recreate the image
        // TODO: Just hash the T#?
        if (tex->width != width || tex->height != height || tex->tsharp.data_format != tsharp->data_format || tex->tsharp.num_format != tsharp->num_format) {
            tex->tsharp = *tsharp;
            tex->width = width;
            tex->height = height;
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
        vk::raii::Buffer tex_buf = vk::raii::Buffer(device, { .size = img_size, .usage = vk::BufferUsageFlagBits::eTransferSrc, .sharingMode = vk::SharingMode::eExclusive });
        auto mem_requirements = tex_buf.getMemoryRequirements();
        vk::raii::DeviceMemory tex_mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
        tex_buf.bindMemory(*tex_mem, 0);
        void* data = tex_mem.mapMemory(0, img_size);
        std::memcpy(data, detiled_buf.get(), img_size);
        tex_mem.unmapMemory();

        // Copy buffer to image
        vk::raii::CommandBuffer tmp_cmd = beginCommands();
        const auto buffer_row_length = pitch > width ? pitch : 0;
        if (pitch < width) printf("pitch < width\n");
        vk::BufferImageCopy region = { .bufferOffset = 0, .bufferRowLength = buffer_row_length, .bufferImageHeight = 0, .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, .imageOffset = { 0, 0, 0 }, .imageExtent = { width, height, 1 } };
        tmp_cmd.copyBufferToImage(*tex_buf, *img, vk::ImageLayout::eTransferDstOptimal, { region });
        endCommands(tmp_cmd);

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

            // TODO: You can in theory change the T# swizzling without changing the texture itself. This breaks because we only hash the texture.
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

    // Check if we are already tracking this texture
    if (tracked_textures.contains(ptr)) {
        auto* tex = tracked_textures[ptr];
        if (tex->dirty) {
            // If the texture was modified, reupload it, reset dirty flag and protect again
            reupload_tex(tex, false);
            tex->dirty = false;
            tex->protect();
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

    reupload_tex(tex, true);
    tracked_textures[ptr] = tex;
    *out_info = &tex->image_info;

    // Protect texture to handle write exceptions
    tex->protect();
}

} // End namespace PS4::GCN::Vulkan