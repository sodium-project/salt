#pragma once
#include <salt/foundation/memory/debugging.hpp>
#include <salt/foundation/utility/terminate.hpp>

#include <salt/platform/win32/api.hpp>

namespace salt::memory::detail {

#if SALT_HAS_ATTRIBUTE(NONNULL)
[[gnu::returns_nonnull]]
#endif
inline void*
win32_heapalloc_common(std::size_t size, std::uint_least32_t flag) noexcept {
    if (0 == size)
        size = 1u;

    auto* memory = win32::HeapAlloc(win32::GetProcessHeap(), flag, size);
    if (!memory) [[unlikely]]
        utility::terminate();

    return memory;
}

#if SALT_HAS_ATTRIBUTE(NONNULL)
[[gnu::returns_nonnull]]
#endif
inline void*
win32_heaprealloc_common(void* address, std::size_t size, std::uint_least32_t flag) noexcept {
    if (0 == size)
        size = 1u;
    if (!address) [[unlikely]]
        return win32::HeapAlloc(win32::GetProcessHeap(), flag, size);

    auto* memory = win32::HeapReAlloc(win32::GetProcessHeap(), flag, address, size);
    if (!memory) [[unlikely]]
        utility::terminate();

    return memory;
}

// clang-format off
// Low-level allocator.
struct [[nodiscard]] win32_heap_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline auto info() noexcept {
        return allocator_info{"salt:memory::detail::win32_heap_allocator", nullptr};
    }

#if SALT_HAS_ATTRIBUTE(MALLOC)
    [[gnu::malloc]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        return win32_heapalloc_common(size, 0u);
    }

    static inline void* reallocate(void* memory, size_type size) noexcept {
        return win32_heaprealloc_common(memory, size, 0u);
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory)
            return;

        win32::HeapFree(win32::GetProcessHeap(), 0u, memory);
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return 0xFFFFFFFFFFFFFFE0;
    }
};
// clang-format on

} // namespace salt::memory::detail