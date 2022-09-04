#pragma once
#include <salt/foundation/fast_terminate.hpp>
#include <salt/memory/debugging.hpp>

namespace salt::detail {

namespace mimalloc {

#if (defined(_MSC_VER) && defined(SALT_CLANG)) || defined(SALT_GNUC)
#    define __cdecl
#endif

// clang-format off
#if defined(_MSC_VER) && !defined(SALT_CLANG)
__declspec(dllimport)
#elif __has_cpp_attribute(__gnu__::__dllimport__)
[[__gnu__::__dllimport__]]
#endif
#if __has_cpp_attribute(__gnu__::__cdecl__)
[[__gnu__::__cdecl__]]
#endif
#if __has_cpp_attribute(__gnu__::__malloc__)
[[__gnu__::__malloc__]]
#endif
extern void* __cdecl mi_malloc(std::size_t) noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
__asm__("mi_malloc")
#endif
;

#if defined(_MSC_VER) && !defined(SALT_CLANG)
__declspec(dllimport)
#elif __has_cpp_attribute(__gnu__::__dllimport__)
[[__gnu__::__dllimport__]]
#endif
#if __has_cpp_attribute(__gnu__::__cdecl__)
[[__gnu__::__cdecl__]]
#endif
extern void __cdecl mi_free(void*) noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
__asm__("mi_free")
#endif
;
// clang-format on

} // namespace mimalloc

// clang-format off
struct [[nodiscard]] Mimalloc_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline auto info() noexcept {
        return Allocator_info{"salt::detail::Mimalloc_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        void* memory = mimalloc::mi_malloc(size);
        if (!memory)
            salt::fast_terminate();
        return memory;
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        mimalloc::mi_free(memory);
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return PTRDIFF_MAX / sizeof(std::byte);
    }
};
// clang-format on

} // namespace salt::detail