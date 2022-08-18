#pragma once
#if defined(_MSC_VER) && !defined(SALT_CLANG)
#    include <cstdlib>
#endif

namespace salt {

[[noreturn]] inline void fast_terminate() noexcept {
// @see: https://llvm.org/doxygen/Compiler_8h_source.html
#if defined(__has_builtin)
#    if __has_builtin(__builtin_trap)
    __builtin_trap();
#    elif __has_builtin(__builtin_abort)
    __builtin_abort();
#    else
    std::abort();
#    endif
#else
    std::abort();
#endif
}

} // namespace salt
