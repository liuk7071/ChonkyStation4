#include "TextureCache.hpp"
#include <Logger.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/TSharp.hpp>
#include <GCN/Detiler/gpuaddr.h>
#include <GCN/Detiler/gnm/texture.h>
#include <xxhash.h>
#include <unordered_map>


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

void getVulkanImageInfoForTSharp(TSharp* tsharp, vk::DescriptorImageInfo** out_info) {
    const u32 width = tsharp->width + 1;
    const u32 height = tsharp->height + 1;
    const u32 pitch = tsharp->pitch + 1;

    auto [vk_fmt, pixel_size] = getBufFormatAndSize(tsharp->data_format, tsharp->num_format);
    void* ptr = (void*)(tsharp->base_address << 8);

    size_t img_size;
    if (vk_fmt == vk::Format::eBc3UnormBlock) {
        const auto blk_width  = (width  + 3) / 4;
        const auto blk_height = (height + 3) / 4;
        img_size = blk_width * blk_height * 16;

    } else img_size = pitch * height * pixel_size;
    
    // Hash the texture
    u64 hash = XXH3_64bits(ptr, img_size);
    // Check if we already have the texture in our cache
    if (cache.contains(hash)) {
        // We already have the texture, return its info
        *out_info = &cache[hash]->image_info;
        return;
    }

    // Check if we already had a texture at this address. If we did, replace it.
    // This is to avoid leaking memory in games like Sonic Mania which are software rendered.
    // Without this, a new texture would get uploaded every frame.
    if (addr_hash_map.contains(ptr)) {
        const auto old_hash = addr_hash_map[ptr];
        delete cache[old_hash];
        cache.erase(old_hash);
    }
    addr_hash_map[ptr] = hash;

    // The texture was not hashed - create it and add it to the cache
    log("Caching new texture\n");
    log("texture size: width=%lld, height=%lld\n", (u32)tsharp->width + 1, (u32)tsharp->height + 1);
    log("texture ptr: %p\n", (void*)tsharp->base_address);
    log("texture dfmt: %d\n", (u32)tsharp->data_format);
    log("texture nfmt: %d\n", (u32)tsharp->num_format);
    log("texture pitch: %d\n", (u32)tsharp->pitch + 1);

    // Detile the texture
    auto detiled_buf = std::make_unique<u8[]>(img_size);
    const GpaTextureInfo tex_info = gnmTexBuildInfo((GnmTexture*)tsharp);
    GpaError err = gpaTileTextureAll(ptr, img_size, detiled_buf.get(), img_size, &tex_info, GNM_TM_DISPLAY_LINEAR_GENERAL);

    //std::ofstream out;
    //out.open(std::format("{}_{}_{}_{}.bin", width, height, pitch, (tsharp->base_address << 8)), std::ios::binary);
    //out.write((char*)(tsharp->base_address << 8), img_size);

    vk::raii::Buffer tex_buf = vk::raii::Buffer(device, { .size = img_size, .usage = vk::BufferUsageFlagBits::eTransferSrc, .sharingMode = vk::SharingMode::eExclusive });
    auto mem_requirements = tex_buf.getMemoryRequirements();
    vk::raii::DeviceMemory tex_mem = vk::raii::DeviceMemory(device, { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) });
    tex_buf.bindMemory(*tex_mem, 0);
    void* data = tex_mem.mapMemory(0, img_size);
    std::memcpy(data, detiled_buf.get(), img_size);
    tex_mem.unmapMemory();

    // Create image
    TextureCacheEntry* entry = new TextureCacheEntry();
    auto& img = entry->image;
    auto& mem = entry->mem;
    vk::ImageCreateInfo img_info = {
        .imageType = vk::ImageType::e2D,
        .format = vk_fmt,
        .extent = { width, height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eLinear,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .sharingMode = vk::SharingMode::eExclusive
    };
    img = vk::raii::Image(device, img_info);
    mem_requirements = img.getMemoryRequirements();
    vk::MemoryAllocateInfo alloc_info = { .allocationSize = mem_requirements.size, .memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal) };
    mem = vk::raii::DeviceMemory(device, alloc_info);
    img.bindMemory(*mem, 0);
    transitionImageLayout(img, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    // Copy buffer to image
    vk::raii::CommandBuffer tmp_cmd = beginCommands();
    vk::BufferImageCopy region = { .bufferOffset = 0, .bufferRowLength = pitch, .bufferImageHeight = 0, .imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, .imageOffset = { 0, 0, 0 }, .imageExtent = { width, height, 1 } };
    tmp_cmd.copyBufferToImage(*tex_buf, *img, vk::ImageLayout::eTransferDstOptimal, { region });
    endCommands(tmp_cmd);

    transitionImageLayout(img, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    // Create image view
    auto& img_view = entry->view;
    vk::ImageViewCreateInfo view_info = { .image = *img, .viewType = vk::ImageViewType::e2D, .format = vk_fmt, .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    img_view = vk::raii::ImageView(device, view_info);

    // Create image sampler
    auto& sampler = entry->sampler;
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
        .compareOp = vk::CompareOp::eAlways,
    };
    sampler = vk::raii::Sampler(device, sampler_info);

    entry->image_info = {
        .sampler = *sampler,
        .imageView = *img_view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
    cache[hash] = entry;
    *out_info = &entry->image_info;
}

} // End namespace PS4::GCN::Vulkan