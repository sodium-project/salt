#pragma once
#include <salt/foundation/types.hpp>

namespace salt::fdn {

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T* addressof(T& arg) noexcept {
    return __builtin_addressof(arg);
}

template <typename T>
T const* addressof(T const&&) = delete;

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] constexpr T* launder(T* p) noexcept {
    return __builtin_launder(p);
}

} // namespace salt::fdn
