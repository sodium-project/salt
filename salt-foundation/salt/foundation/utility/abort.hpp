#pragma once
#if defined(_MSC_VER) && !defined(SALT_CLANG)
#    include <cstdlib>
#endif

namespace salt::utility {

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

} // namespace salt::utility
