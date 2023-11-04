#pragma once
#include <salt/config.hpp>

namespace salt::memory {

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T* addressof(T& arg) noexcept {
    return __builtin_addressof(arg);
}

template <typename T>
T const* addressof(T const&&) = delete;

} // namespace salt::memory