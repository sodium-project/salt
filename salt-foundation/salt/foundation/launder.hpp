#pragma once

namespace salt::fdn {

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T* launder(T* p) noexcept {
    return __builtin_launder(p);
}

} // namespace salt::fdn