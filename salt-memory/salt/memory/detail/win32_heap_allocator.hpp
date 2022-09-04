#pragma once
#include <salt/foundation/fast_terminate.hpp>
#include <salt/memory/debugging.hpp>

namespace salt::detail {

namespace win32 {

// clang-format off
#if defined(_MSC_VER) && !defined(SALT_CLANG)
__declspec(dllimport)
#elif __has_cpp_attribute(__gnu__::__dllimport__)
[[__gnu__::__dllimport__]]
#endif
#if __has_cpp_attribute(__gnu__::__malloc__)
[[__gnu__::__malloc__]]
#endif
extern void* __stdcall HeapAlloc(void*, std::uint32_t, std::size_t) noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
__asm__("HeapAlloc")
#endif
;

#if defined(_MSC_VER) && !defined(SALT_CLANG)
__declspec(dllimport)
#elif __has_cpp_attribute(__gnu__::__dllimport__)
[[__gnu__::__dllimport__]]
#endif
extern int __stdcall HeapFree(void*, std::uint32_t, void*) noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
__asm__("HeapFree")
#endif
;

#if defined(_MSC_VER) && !defined(SALT_CLANG)
__declspec(dllimport)
#elif __has_cpp_attribute(__gnu__::__dllimport__)
[[__gnu__::__dllimport__]]
#endif
#if __has_cpp_attribute(__gnu__::__const__)
[[__gnu__::__const__]]
#endif
extern void* __stdcall GetProcessHeap() noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
__asm__("GetProcessHeap")
#endif
;
// clang-format on

} // namespace win32

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
[[__gnu__::__returns_nonnull__]]
#endif
inline void*
win32_heapalloc_common_impl(std::size_t size, std::uint32_t flag) noexcept {
    auto* memory = win32::HeapAlloc(win32::GetProcessHeap(), flag, size ? size : 1);
    if (!memory)
        salt::fast_terminate();
    return memory;
}

// clang-format off
struct [[nodiscard]] Win32_heap_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline auto info() noexcept {
        return Allocator_info{"salt::detail::Win32_heap_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__malloc__)
    [[__gnu__::__malloc__]]
#endif
    static inline void* allocate(size_type size, size_type) noexcept {
        return win32_heapalloc_common_impl(size, 0u);
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory) [[unlikely]]
            return;

        win32::HeapFree(win32::GetProcessHeap(), 0u, memory);
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return 0xFFFFFFFFFFFFFFE0;
    }
};
// clang-format on

} // namespace salt::detail