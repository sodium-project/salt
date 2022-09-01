#pragma once
#include <cstddef>
#include <cstdint>

#include <salt/config/compiler_support.hpp>
#include <salt/foundation/fast_terminate.hpp>
#include <salt/memory/debugging.hpp>

namespace salt::detail {

namespace mimalloc {

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
extern void* SALT_CDECL mi_malloc(std::size_t) noexcept
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
extern void SALT_CDECL mi_free(void*) noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
__asm__("mi_free")
#endif
;
// clang-format on

} // namespace mimalloc

// clang-format off
struct [[nodiscard]] Mimalloc_allocator final {

    static inline Allocator_info info() noexcept {
        return {"salt::detail::Mimalloc_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#endif
    static inline void* allocate(std::size_t size, std::size_t) noexcept {
        void* memory = mimalloc::mi_malloc(size);
        if (!memory)
            salt::fast_terminate();
        return memory;
    }

    static inline void deallocate(void* ptr, std::size_t, std::size_t) noexcept {
        mimalloc::mi_free(ptr);
    }

    static inline std::size_t max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return PTRDIFF_MAX / sizeof(std::byte);
    }
};
// clang-format on

} // namespace salt::detail