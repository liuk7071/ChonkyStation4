#include "BufferCache.hpp"
#include <Logger.hpp>
#include <GCN/Backends/Vulkan/VulkanCommon.hpp>
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
    vk::Buffer      buf = nullptr;
    size_t          offs_in_buf = 0;
    void* mapping = nullptr;
    VmaAllocation   alloc;

    void protect() {
#ifdef _WIN32
        const auto aligned_start = Helpers::alignDown<u64>((u64)base, page_size);
        const auto aligned_end = Helpers::alignUp<u64>((u64)base + size, page_size);

        DWORD old_protect;
        if (!VirtualProtect((void*)aligned_start, aligned_end - aligned_start, PAGE_READONLY, &old_protect))
            Helpers::panic("CachedBuffer::protect: VirtualProtect failed");
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
            Helpers::panic("TrackedRegion::protect: VirtualProtect failed");
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
std::unordered_map<u64, CachedBuffer*> hash_cache;

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
        cache[page]->dirty = true;
        cache[page]->unprotect();
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
std::vector<Allocation> allocations;

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

    allocations.reserve(4096);
}

void updateBuffer(CachedBuffer* buf) {
    auto& vk_buf = buf->buf;
    auto& alloc = buf->alloc;

    // We create a new buffer instead of actually updating the old one,
    // and then we free all allocations when clear() is called (at the end of every frame), in stream-buffer fashion.
    // This is because buffers can and will be updated mid-frame, so we can't free the old ones right away.
    // The allocation costs shouldn't be too much due to how VMA is setup - see VulkanRenderer.cpp
    const vk::BufferCreateInfo buf_create_info = {
        .size = buf->size,
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
    vmaCreateBuffer(allocator, &*buf_create_info, &alloc_create_info, &raw_buf, &alloc, &info);
    vk_buf = vk::Buffer(raw_buf);

    // Update the buffer
    std::memcpy(info.pMappedData, (void*)buf->base, buf->size);
    allocations.push_back({ .buf = vk_buf, .alloc = alloc });
}

std::tuple<vk::Buffer, size_t, bool> getBuffer(void* base, size_t size) {
    const uptr   aligned_base   = Helpers::alignDown<uptr>((uptr)base, page_size);
    const uptr   aligned_end    = Helpers::alignUp<uptr>((uptr)base + size, page_size);
    const size_t aligned_size   = aligned_end - aligned_base;
    const u64    size_in_pages  = aligned_size >> page_bits;
    const u64    page           = aligned_base >> page_bits;
    const u64    page_end       = page + size_in_pages;
    
    auto lk = std::unique_lock<std::mutex>(cache_mtx);

    // Check if we already cached this buffer
    if (size >= page_size) {
        if (cache.contains(page)) {
            // Check if we need to reupload the buffer
            auto* buf = cache[page];
            const bool size_changed = page_end > buf->page_end;
            const bool was_dirty = buf->dirty || size_changed;

            if (buf->dirty || size_changed) {
                if (size_changed) {
                    buf->page_end = page_end;
                    // Update page table
                    buf->size = (page_end - buf->page) << page_bits;
                    for (int i = 0; i < buf->page_end - buf->page; i++)
                        cache[buf->page + i] = buf;
                }

                buf->dirty = false;
                updateBuffer(buf);
                buf->protect();
            }

            // Return the buffer
            return { buf->buf, (uptr)base - (uptr)buf->base, was_dirty };
        }
    }
    else {
        const u64 hash = XXH3_64bits(base, size);
        if (hash_cache.contains(hash)) {
            auto* buf = hash_cache[hash];
            return { buf->buf, 0, false };
        }
    }

    // The buffer is new - create and cache it
    CachedBuffer* buf = new CachedBuffer();
    if (size >= page_size) {
        buf->base = (void*)aligned_base;
        buf->page = page;
        buf->page_end = page_end;
        buf->size = aligned_size;
        for (u64 i = 0; i < size_in_pages; i++)
            cache[page + i] = buf;
        buf->protect();
    }
    else {
        buf->base = base;
        buf->size = size;
        buf->hash = XXH3_64bits(base, size);
        hash_cache[buf->hash] = buf;
    }

    // TODO: Page faulting doesn't work correctly on Windows when the game code uses the stack's red zone (negative rsp offsets), because this is not allowed on Windows
    // and the kernel's exception handling stuff ends up corrupting it (it pushes data on the same stack as the faulting instruction).
    // As a workaround, for buffers smaller than 1 page we fallback to hashing to reduce the chances of hitting exceptions near red zone code.
    // There is no fix for this because the corruption happens before my exception handler is even called.
    updateBuffer(buf);
    return { buf->buf, (uptr)base - (uptr)buf->base, true };
}

// This function exists to allow us to track memory pages without necessarily tying them to a Vulkan buffer.
// I use this in my texture cache, where I handle the Vulkan code separately.
// TODO: If you overwrite a region, the callback is silently not updated.
// Change this behavior if I ever use this anywhere other than the texture cache.
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

// Returns true if at least page in the specified region is dirty, otherwise false.
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

    for (auto& alloc : allocations)
        vmaDestroyBuffer(allocator, alloc.buf, alloc.alloc);
    allocations.clear();

    for (auto& buf : cache)
        buf.second->dirty = true;
    hash_cache.clear();
}

}   // End namespace PS4::GCN::Vulkan::Cache