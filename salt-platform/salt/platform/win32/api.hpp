#pragma once
#include <salt/config.hpp>

#include <cstddef>
#include <cstdint>

namespace salt::win32 {

// clang-format off
#if SALT_HAS_ATTRIBUTE(DLLIMPORT)
[[gnu::dllimport]]
#endif
#if SALT_HAS_ATTRIBUTE(STDCALL)
[[gnu::stdcall]]
#endif
extern void* GetStdHandle(std::uint_least32_t) noexcept
#if defined(SALT_CLANG)
__asm__("GetStdHandle")
#endif
;

#if SALT_HAS_ATTRIBUTE(DLLIMPORT)
[[gnu::dllimport]]
#endif
#if SALT_HAS_ATTRIBUTE(STDCALL)
[[gnu::stdcall]]
#endif
extern int SetConsoleMode(void*, std::uint_least32_t) noexcept
#if defined(SALT_CLANG) 
__asm__("SetConsoleMode")
#endif
;

#if SALT_HAS_ATTRIBUTE(DLLIMPORT)
[[gnu::dllimport]]
#endif
#if SALT_HAS_ATTRIBUTE(STDCALL)
[[gnu::stdcall]]
#endif
#if SALT_HAS_ATTRIBUTE(MALLOC)
[[gnu::malloc]]
#endif
extern void* HeapAlloc(void*, std::uint_least32_t, std::size_t) noexcept
#if defined(SALT_CLANG)
__asm__("HeapAlloc")
#endif
;

#if SALT_HAS_ATTRIBUTE(DLLIMPORT)
[[gnu::dllimport]]
#endif
#if SALT_HAS_ATTRIBUTE(STDCALL)
[[gnu::stdcall]]
#endif
extern void* HeapFree(void*, std::uint_least32_t, void*) noexcept
#if defined(SALT_CLANG)
__asm__("HeapFree")
#endif
;

#if SALT_HAS_ATTRIBUTE(DLLIMPORT)
[[gnu::dllimport]]
#endif
#if SALT_HAS_ATTRIBUTE(STDCALL)
[[gnu::stdcall]]
#endif
#if SALT_HAS_ATTRIBUTE(CONST)
[[gnu::const]]
#endif
extern void* GetProcessHeap() noexcept
#if defined(SALT_CLANG)
__asm__("GetProcessHeap")
#endif
;

#if SALT_HAS_ATTRIBUTE(DLLIMPORT)
[[gnu::dllimport]]
#endif
#if SALT_HAS_ATTRIBUTE(STDCALL)
[[gnu::stdcall]]
#endif
extern void* HeapReAlloc(void*, std::uint_least32_t, void*, std::size_t) noexcept
#if defined(SALT_CLANG)
__asm__("HeapReAlloc")
#endif
;
// clang-format on

} // namespace salt::win32