#pragma once
#include <cstddef>
#include <cstdint>

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

#if defined(_MSC_VER) && !defined(SALT_CLANG)
__declspec(dllimport)
#elif __has_cpp_attribute(__gnu__::__dllimport__)
[[__gnu__::__dllimport__]]
#endif
extern void* __stdcall HeapReAlloc(void*, std::uint32_t, void*, std::size_t) noexcept
#if defined(SALT_CLANG) || defined(SALT_GNUC)
    __asm__("HeapReAlloc")
#endif
;
// clang-format on

} // namespace win32

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
[[__gnu__::__returns_nonnull__]]
#endif
inline void*
win32_heapalloc_common_impl(std::size_t size, std::uint32_t flag) noexcept {
    auto* ptr = win32::HeapAlloc(win32::GetProcessHeap(), flag, size ? size : 1);
    if (!ptr)
        salt::fast_terminate();
    return ptr;
}

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
[[__gnu__::__returns_nonnull__]]
#endif
inline void*
win32_heaprealloc_common_impl(void* ptr, std::size_t size, std::uint32_t flag) noexcept {
    if (!ptr) [[unlikely]]
        return win32_heapalloc_common_impl(size, flag);

    auto* memory = win32::HeapReAlloc(win32::GetProcessHeap(), flag, ptr, size ? size : 1);
    if (!memory)
        salt::fast_terminate();
    return memory;
}

// clang-format off
struct [[nodiscard]] Win32_heap_allocator final {

    static inline Allocator_info info() noexcept {
        return {"salt::detail::Win32_heap_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__malloc__)
    [[__gnu__::__malloc__]]
#endif
    static inline void* allocate(std::size_t size, std::size_t) noexcept {
        return win32_heapalloc_common_impl(size, 0u);
    }

    static inline void deallocate(void* ptr, std::size_t, std::size_t) noexcept {
        if (!ptr)
            return;

        win32::HeapFree(win32::GetProcessHeap(), 0u, ptr);
    }

    static inline std::size_t max_size() noexcept {
        return 0xFFFFFFFFFFFFFFE0;
    }
};
// clang-format on

} // namespace salt::detail