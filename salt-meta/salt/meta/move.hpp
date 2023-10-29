#pragma once
#include <salt/config.hpp>

namespace salt::meta {

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr meta::remove_ref_t<T>&& move(T&& t) noexcept {
    return static_cast<meta::remove_ref_t<T>&&>(t);
}

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T&& forward(meta::remove_ref_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}

template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
[[nodiscard]] inline constexpr T&& forward(meta::remove_ref_t<T>&& t) noexcept {
    static_assert(not is_lvalue_reference_v<T>, "cannot forward an rvalue as an lvalue");
    return static_cast<T&&>(t);
}

using std::forward_as_tuple;

} // namespace salt::meta