#pragma once
#if defined(_MSC_VER) && !defined(SALT_CLANG)
#    include <cstdlib>
#endif

namespace salt::fdn {

[[noreturn]] inline void unreachable() noexcept {
#if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
#else
    __assume(false);
#endif
}

} // namespace salt::fdn
