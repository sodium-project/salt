#pragma once
#include <salt/config.hpp>
#include <salt/foundation/types.hpp>

namespace salt::win32 {

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

} // namespace salt::win32