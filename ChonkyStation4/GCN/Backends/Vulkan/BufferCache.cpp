#include "BufferCache.hpp"
#include <Logger.hpp>
#include <Profiler.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
#include <GCN/GCN.hpp>
#include <xxhash.h>
#include <unordered_map>
#include <mutex>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <intrin.h>
#endif


namespace PS4::GCN::Vulkan::Cache {

MAKE_LOG_FUNCTION(log, gcn_vulkan_renderer);

struct CachedBuffer {
    void* base = nullptr;
    size_t  size = 0;
    u64     page = 0;
    u64     page_end = 0;
    u64     hash = 0;
    bool    dirty = false;
    std::vector<bool> dirty_pages;
    vk::Buffer      staging_buf = nullptr;
    vk::Buffer      buf = nullptr;
    size_t          offs_in_buf = 0;
    void* mapping = nullptr;
    VmaAllocation   staging_alloc;
    VmaAllocation   alloc;
    //u64 last_used_frame = 0;
    //u64 last_base_used_frame = 0;
    //std::vector<u64> last_used_page_frame;

    void protect(u64 page_to_protect) {
#ifdef _WIN32
        DWORD old_protect;
        if (!VirtualProtect((void*)((page + page_to_protect) << page_bits), page_size, PAGE_READONLY, &old_protect))
            Helpers::panic("CachedBuffer::protect: VirtualProtect failed");
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }

    void unprotect(u64 page_to_unprotect) {
#ifdef _WIN32
        DWORD old_protect;
        if (!VirtualProtect((void*)((page + page_to_unprotect) << page_bits), page_size, PAGE_READWRITE, &old_protect))
            Helpers::panic("CachedBuffer::unprotect: VirtualProtect failed");
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }
};

struct TrackedRegion {
    void* base = nullptr;
    size_t  size = 0;
    u64     page = 0;
    u64     page_end = 0;
    bool    dirty = false;
    std::function<void(uptr)> callback;

    void protect() {
#ifdef _WIN32
        const auto aligned_start = Helpers::alignDown<u64>((u64)base, page_size);
        const auto aligned_end = Helpers::alignUp<u64>((u64)base + size, page_size);

        DWORD old_protect;
        if (!VirtualProtect((void*)aligned_start, aligned_end - aligned_start, PAGE_READONLY, &old_protect))
            //Helpers::panic("TrackedRegion::protect: VirtualProtect failed");
            printf("TrackedRegion::protect: VirtualProtect failed at address 0x%llx\n", aligned_start);
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }

    void unprotect() {
#ifdef _WIN32
        const auto aligned_start = Helpers::alignDown<u64>((u64)base, page_size);
        const auto aligned_end = Helpers::alignUp<u64>((u64)base + size, page_size);

        DWORD old_protect;
        if (!VirtualProtect((void*)aligned_start, aligned_end - aligned_start, PAGE_READWRITE, &old_protect))
            Helpers::panic("TrackedRegion::unprotect: VirtualProtect failed");
#else
        Helpers::panic("Unsupported platform\n");
#endif
    }
};

std::mutex cache_mtx;
std::unordered_map<u64, CachedBuffer*> cache;
std::unordered_map<u64, TrackedRegion*> tracked;
std::unordered_map<u64, CachedBuffer*> hash_cache[FRAMES_IN_FLIGHT];

#ifdef _WIN32

static LONG CALLBACK exceptionHandler(EXCEPTION_POINTERS* info) noexcept {
    const auto* record = info->ExceptionRecord;
    if (record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
        return EXCEPTION_CONTINUE_SEARCH;

    const bool is_write = record->ExceptionInformation[0] == 1;
    if (!is_write) return EXCEPTION_CONTINUE_SEARCH;

    void* addr = (void*)record->ExceptionInformation[1];
    const u64 page = (uptr)addr >> page_bits;

    auto lk = std::unique_lock<std::mutex>(cache_mtx);
    bool handled = false;

    if (cache.contains(page)) {
        handled = true;

        auto& buf = cache[page];
        buf->dirty = true;
        buf->dirty_pages[page - buf->page] = true;
        cache[page]->unprotect(page - buf->page);

        //printf("addr %p base %p size %lld last used (all/base) %d/%d frames ago\n", addr, buf->base, buf->size, GCN::global_flip_counter - buf->last_used_frame, GCN::global_flip_counter - buf->last_base_used_frame);
        //printf("page was bound %d frames ago\n", GCN::global_flip_counter - buf->last_used_page_frame[page - buf->page]);
    }

    if (tracked.contains(page)) {
        handled = true;
        tracked[page]->dirty = true;
        tracked[page]->callback((uptr)addr);
        tracked[page]->unprotect();
    }

    return handled ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

#endif

struct Allocation {
    vk::Buffer buf;
    VmaAllocation alloc;
};
std::vector<Allocation> allocations_to_clear[FRAMES_IN_FLIGHT];

void init() {
    // Setup exception handler
#ifdef _WIN32
    if (!AddVectoredExceptionHandler(0, exceptionHandler))
        Helpers::panic("initTextureCache: failed to register exception handler");

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    page_size = si.dwPageSize;
    page_bits = std::bit_width(page_size - 1);
#else
    Helpers::panic("Unsupported platform\n");
#endif

    for (auto& allocations : allocations_to_clear)
        allocations.reserve(10000);
}

void updateBuffer(CachedBuffer* buf, bool recreate_vk_buf, u64 starting_page = 0) {
    //Profiler::Scope profiler("updateBuffer");
    auto& staging_vk_buf    = buf->staging_buf;
    auto& vk_buf            = buf->buf;
    auto& staging_alloc     = buf->staging_alloc;
    auto& alloc             = buf->alloc;

    // We create a new buffer instead of actually updating the old one,
    // and then we free all allocations when clear() is called (at the end of every frame), in stream-buffer fashion(?).
    // This is because buffers can and will be updated mid-frame, so we can't free the old ones right away.
    // The allocation costs shouldn't be too much due to how VMA is setup - see VulkanRenderer.cpp

    auto copy_region = [&](void* base, size_t offset, size_t size, vk::Buffer dest_vk_buf) {
        const vk::BufferCreateInfo buf_create_info = {
            .size = size,
            .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer
                     | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst
                     | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer
                     | vk::BufferUsageFlagBits::eIndirectBuffer,
            .sharingMode = vk::SharingMode::eExclusive
        };
        VmaAllocationCreateInfo alloc_create_info = { .pool = vma_pool };
        alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VkBuffer raw_buf;
        VmaAllocationInfo info;
        vmaCreateBuffer(allocator, &*buf_create_info, &alloc_create_info, &raw_buf, &staging_alloc, &info);
        staging_vk_buf = vk::Buffer(raw_buf);

        if (info.size < size) {
            Helpers::panic("Cache::updateBuffer: could not allocate full buffer");
        }

        // Update the buffer
        std::memcpy(info.pMappedData, (u8*)base + offset, size);
    
        // Copy staging buffer to device local buffer
        endRendering();
        cmd_bufs[frame_idx].copyBuffer(staging_vk_buf, dest_vk_buf, vk::BufferCopy { 0, offset, size });

        // Clear staging buffer after this frame
        allocations_to_clear[frame_idx].push_back({ .buf = staging_vk_buf, .alloc = staging_alloc });
    };

    // Recreate and copy the whole device local buffer if needed
    if (recreate_vk_buf) {
        if (vk_buf) {
            allocations_to_clear[frame_idx].push_back({ .buf = vk_buf, .alloc = alloc });
        }

        const vk::BufferCreateInfo buf_create_info = {
            .size = buf->size,
            .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer
                     | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst
                     | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer
                     | vk::BufferUsageFlagBits::eIndirectBuffer,
            .sharingMode = vk::SharingMode::eExclusive
        };

        VmaAllocationCreateInfo alloc_create_info = { .pool = device_vma_pool };
        alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        alloc_create_info.flags = 0;
        VkBuffer raw_buf;
        vmaCreateBuffer(allocator, &*buf_create_info, &alloc_create_info, &raw_buf, &alloc, nullptr);
        vk_buf = vk::Buffer(raw_buf);
        
        // Copy and protect the whole buffer
        copy_region(buf->base, 0, buf->size, vk_buf);
        for (int i = 0; i < buf->page_end - buf->page; i++) {
            buf->protect(i);
        }
    }
    else {
        // Only reupload dirty pages of the buffer
        // Merge neighboring pages together instead of doing individual page uploads
        size_t page = starting_page;
        auto& dirty_pages = buf->dirty_pages;
        while (page < dirty_pages.size()) {
            // Skip non-dirty pages
            if (!dirty_pages[page]) {
                page++;
                continue;
            }

            const size_t page_start = page;

            // Find page end while re-protecting dirty pages
            while (page < dirty_pages.size() && dirty_pages[page]) {
                buf->protect(page);
                page++;
            }

            const size_t page_end = page;
            const size_t size = (page_end - page_start) << page_bits;
            copy_region(buf->base, page_start << page_bits, size, vk_buf);
        }
    }
    
    VkBufferMemoryBarrier barrier{
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        vk_buf,
        0,
        buf->size
    };
    
    vkCmdPipelineBarrier(
        *cmd_bufs[frame_idx],
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        1, &barrier,
        0, nullptr
    );
}

void deleteBuf(CachedBuffer* buf) {
    for (u64 i = 0; i < buf->page_end - buf->page; i++) {
        auto it = cache.find(buf->page + i);
        if (it != cache.end() && it->second == buf) {
            buf->unprotect(i);
            cache.erase(it);
        }
    }
    if (buf->buf)
        allocations_to_clear[frame_idx].push_back({ .buf = buf->buf, .alloc = buf->alloc });
    delete buf;
}

std::tuple<vk::Buffer, size_t, bool> getBuffer(void* base, size_t size) {
    const uptr   aligned_base   = Helpers::alignDown<uptr>((uptr)base, page_size);
    const uptr   aligned_end    = Helpers::alignUp<uptr>((uptr)base + size, page_size);
    const size_t aligned_size   = aligned_end - aligned_base;
    const u64    size_in_pages  = aligned_size >> page_bits;
    const u64    page           = aligned_base >> page_bits;
    const u64    page_end       = page + size_in_pages;
    
    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    const bool is_hash = size < page_size / 4;

    // Check if we already cached this buffer
    if (!is_hash) {
        auto it = cache.find(page);
        if (it != cache.end()) {
            // Check if we need to reupload the buffer
            auto* buf = it->second;
            const bool size_changed = page_end > buf->page_end;
            const bool was_dirty = buf->dirty || size_changed;

            if (buf->dirty || size_changed) {
                if (size_changed) {
                    buf->page_end = page_end;
                    buf->size = (page_end - buf->page) << page_bits;
                    
                    // Update page table
                    for (int i = 0; i < buf->page_end - buf->page; i++) {
                        auto it = cache.find(buf->page + i);
                        if (it != cache.end() && it->second != buf)
                            deleteBuf(it->second);
                        cache[buf->page + i] = buf;
                    }
                    
                    buf->dirty_pages.resize(buf->page_end - buf->page);
                    //buf->last_used_page_frame.resize(buf->page_end - buf->page);
                }

                updateBuffer(buf, size_changed);    // Will re-protect dirty pages
                buf->dirty = false;
                std::fill(buf->dirty_pages.begin(), buf->dirty_pages.end(), false); // DONT .clear(), because that resets the size to 0
            }

            // Return the buffer
            //buf->last_used_frame = GCN::global_flip_counter;
            //if (buf->base == base)
            //    buf->last_base_used_frame = GCN::global_flip_counter;
            //for (u64 p = 0; p < size_in_pages; p++)
            //    buf->last_used_page_frame[page - ((uptr)buf->base >> Cache::page_bits) + p] = GCN::global_flip_counter;
            
            return { buf->buf, (uptr)base - (uptr)buf->base, was_dirty };
        }
    }
    else {
        const u64 hash = XXH3_64bits(base, size);
        if (hash_cache[frame_idx].contains(hash)) {
            auto* buf = hash_cache[frame_idx][hash];
            return { buf->buf, 0, false };
        }
    }

    // The buffer is new - create and cache it
    CachedBuffer* buf = new CachedBuffer();
    if (!is_hash) {
        buf->base = (void*)aligned_base;
        buf->page = page;
        buf->page_end = page_end;
        buf->size = aligned_size;
        buf->dirty_pages.resize(size_in_pages);
        //buf->last_used_page_frame.resize(size_in_pages);
        //buf->last_used_frame = GCN::global_flip_counter;
        //buf->last_base_used_frame = GCN::global_flip_counter;

        for (u64 i = 0; i < size_in_pages; i++) {
            auto it = cache.find(page + i);
            if (it != cache.end() && it->second != buf)
                deleteBuf(it->second);

            cache[page + i] = buf;
            buf->protect(i);

            //buf->last_used_page_frame[i] = GCN::global_flip_counter;
        }
    }
    else {
        buf->base = base;
        buf->size = size;
        buf->hash = XXH3_64bits(base, size);
        hash_cache[frame_idx][buf->hash] = buf;
    }

    // TODO: Page faulting doesn't work correctly on Windows when the game code uses the stack's red zone (negative rsp offsets), because this is not allowed on Windows
    // and the kernel's exception handling stuff ends up corrupting it (it pushes data on the same stack as the faulting instruction).
    // As a workaround, for buffers smaller than 1 page we fallback to hashing to reduce the chances of hitting exceptions near red zone code.
    // There is no fix for this because the corruption happens before my exception handler is even called.
    updateBuffer(buf, true);
    return { buf->buf, (uptr)base - (uptr)buf->base, true };
}

void barrier() {
    VkMemoryBarrier barrier {
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_SHADER_READ_BIT
    };

    vkCmdPipelineBarrier(
        *cmd_bufs[frame_idx],
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        1, &barrier,
        0, nullptr,
        0, nullptr
    );
}

// This function returns an empty mapped buffer that is cleared at the end of the frame (when clear() is called)
std::pair<vk::Buffer, void*> getMappedBufferForFrame(size_t size) {
    const vk::BufferCreateInfo buf_create_info = {
        .size = size,
        .usage =   vk::BufferUsageFlagBits::eIndexBuffer   | vk::BufferUsageFlagBits::eVertexBuffer
                 | vk::BufferUsageFlagBits::eTransferSrc   | vk::BufferUsageFlagBits::eTransferDst
                 | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
        .sharingMode = vk::SharingMode::eExclusive
    };
    VmaAllocationCreateInfo alloc_create_info = { .pool = vma_pool };
    alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VkBuffer raw_buf;
    VmaAllocationInfo info;
    VmaAllocation alloc;
    vmaCreateBuffer(allocator, &*buf_create_info, &alloc_create_info, &raw_buf, &alloc, &info);
    vk::Buffer vk_buf = vk::Buffer(raw_buf);

    allocations_to_clear[frame_idx].push_back({ .buf = vk_buf, .alloc = alloc });
    return { vk_buf, (void*)info.pMappedData };
}

// This function exists to allow us to track memory pages without necessarily tying them to a Vulkan buffer.
// I use this in my texture cache, where I handle the Vulkan code separately.
// TODO: If you overwrite a region, the callback is silently not updated.
// Change this behavior if I ever use this anywhere other than the texture cache (where the callback is always the same so it's not an issue).
void track(void* base, size_t size, std::function<void(uptr)> callback) {
    const uptr   aligned_base = Helpers::alignDown<uptr>((uptr)base, page_size);
    const uptr   aligned_end = Helpers::alignUp<uptr>((uptr)base + size, page_size);
    const size_t aligned_size = aligned_end - aligned_base;
    const u64    size_in_pages = aligned_size >> page_bits;
    const u64    page = aligned_base >> page_bits;
    const u64    page_end = page + size_in_pages;

    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    // Check if we already tracked region
    if (tracked.contains(page)) {
        // Check if we need to re-track due to size changes
        auto* buf = tracked[page];
        const bool size_changed = page_end > buf->page_end;

        if (size_changed) {
            buf->page_end = page_end;
            // Update page table
            buf->size = (page_end - buf->page) << page_bits;
            for (u64 i = 0; i < buf->page_end - buf->page; i++)
                tracked[buf->page + i] = buf;
        }

        buf->protect();
        return;
    }

    // The tracked region is new - create and cache it
    TrackedRegion* buf = new TrackedRegion();
    buf->dirty = false;
    buf->base = (void*)aligned_base;
    buf->page = page;
    buf->page_end = page_end;
    buf->size = aligned_size;
    buf->callback = callback;
    for (u64 i = 0; i < size_in_pages; i++)
        tracked[page + i] = buf;
    buf->protect();
}

// Unprotects a page
void unprotect(u64 page) {
    auto buf = tracked.find(page);
    if (buf != tracked.end())
        buf->second->unprotect();
    else printf("Cache::unprotect: page 0x%llx was not tracked\n", page);
}

// Re-enables memory protection and resets the dirty flag.
bool resetDirty(void* base, size_t size) {
    const uptr   aligned_base = Helpers::alignDown<uptr>((uptr)base, page_size);
    const uptr   aligned_end = Helpers::alignUp<uptr>((uptr)base + size, page_size);
    const size_t aligned_size = aligned_end - aligned_base;
    const u64    size_in_pages = aligned_size >> page_bits;
    const u64    page = aligned_base >> page_bits;
    const u64    page_end = page + size_in_pages;

    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    // Check if we already cached this buffer
    if (tracked.contains(page)) {
        // Check if we need to reupload the buffer
        auto* buf = tracked[page];
        const bool size_changed = page_end > buf->page_end;

        if (size_changed) {
            buf->page_end = page_end;
            // Update page table
            buf->size = (page_end - buf->page) << page_bits;
            for (u64 i = 0; i < buf->page_end - buf->page; i++)
                tracked[buf->page + i] = buf;
        }

        buf->dirty = false;
        buf->protect();
        return true;
    }
    
    return false;
    Helpers::panic("Cache::resetDirty: cache error");
}

// Returns true if at least one page in the specified region is dirty, otherwise false.
bool isDirty(void* base, size_t size) {
    const uptr   aligned_base = Helpers::alignDown<uptr>((uptr)base, page_size);
    const uptr   aligned_end = Helpers::alignUp<uptr>((uptr)base + size, page_size);
    const size_t aligned_size = aligned_end - aligned_base;
    const u64    size_in_pages = aligned_size >> page_bits;
    const u64    page = aligned_base >> page_bits;
    const u64    page_end = page + size_in_pages;

    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    if (tracked.contains(page)) return tracked[page]->dirty;
    else Helpers::panic("Cache::isDirty: cache error");
}

void clear() {
    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    {
        //Profiler::Scope profiler("Buffer cleanup");
        for (auto& alloc : allocations_to_clear[frame_idx]) {
            vmaDestroyBuffer(allocator, alloc.buf, alloc.alloc);
        }
        allocations_to_clear[frame_idx].clear();
    }

    {
        //Profiler::Scope profiler("Buffer hash cache cleanup");
        for (auto& [hash, buf] : hash_cache[frame_idx]) {
            vmaDestroyBuffer(allocator, buf->buf, buf->alloc);
            delete buf;
        }
        hash_cache[frame_idx].clear();
    }
}

}   // End namespace PS4::GCN::Vulkan::Cache