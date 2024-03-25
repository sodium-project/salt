#pragma once

#include <salt/config.hpp>

#include <salt/foundation/debugging.hpp>
#include <salt/foundation/terminate.hpp>

namespace salt::fdn::detail {

// clang-format off
// Low-level allocator.
struct [[nodiscard]] malloc_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline auto info() noexcept {
        return allocator_info{"salt:fdn::detail::malloc_allocator", nullptr};
    }

#if SALT_HAS_ATTRIBUTE(NONNULL)
    [[gnu::returns_nonnull]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        void* memory = 
#    if __has_builtin(__builtin_malloc)
        __builtin_malloc(size);
#    else
        std::malloc(size);
#    endif
        if (!memory) [[unlikely]]
            terminate();

        return memory;
    }

    static inline void* reallocate(void* memory, size_type size) noexcept {
        memory = 
#    if __has_builtin(__builtin_realloc)
        __builtin_realloc(memory, size);
#    else
        std::realloc(memory, size);
#    endif
        if (!memory) [[unlikely]]
            terminate();

        return memory;
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory)
            return;

#    if __has_builtin(__builtin_free)
        __builtin_free(memory);
#    else
        std::free(memory);
#    endif
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return size_type(-1) / sizeof(std::byte);
    }
};
// clang-format on

} // namespace salt::fdn::detail