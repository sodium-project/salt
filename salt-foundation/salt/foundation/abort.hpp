#pragma once
#if defined(_MSC_VER) && !defined(SALT_CLANG)
#    include <cstdlib>
#endif

namespace salt::fdn {

[[noreturn]] inline void abort() noexcept {
// @see: https://llvm.org/doxygen/Compiler_8h_source.html
#if __has_builtin(__builtin_trap)
    __builtin_trap();
#elif __has_builtin(__builtin_abort)
    __builtin_abort();
#else
    std::abort();
#endif
}

[[noreturn]] inline void unreachable() noexcept {
#if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
#else
    __assume(false);
#endif
}

} // namespace salt::fdn
