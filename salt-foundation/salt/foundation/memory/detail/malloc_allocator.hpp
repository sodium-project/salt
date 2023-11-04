#pragma once
#include <salt/foundation/memory/debugging.hpp>

namespace salt::memory::detail {

// clang-format off
// Low-level allocator.
struct [[nodiscard]] malloc_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline auto info() noexcept {
        return allocator_info{"salt:memory::detail::malloc_allocator", nullptr};
    }

#if SALT_HAS_ATTRIBUTE(NONNULL)
    [[gnu::returns_nonnull]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        auto* memory = __builtin_malloc(size);
        if (!memory) [[unlikely]]
            __builtin_trap();

        return memory;
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory)
            return;

        __builtin_free(memory);
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return size_type(-1) / sizeof(std::byte);
    }
};
// clang-format on

} // namespace salt::memory::detail