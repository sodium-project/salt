#pragma once

#include <salt/config.hpp>

#include <salt/foundation/addressof.hpp>
#include <salt/foundation/detail/constexpr_memcpy.hpp>

namespace salt::fdn::detail {

// clang-format off
template <typename T>
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
[[clang::always_inline]]
#endif
constexpr void* voidify(T& from) noexcept {
    // Cast away cv-qualifiers to allow modifying elements of a range through const iterators.
    return const_cast<void*>(static_cast<void const volatile*>(addressof(from)));
}
// clang-format on

} // namespace salt::fdn::detail

// This is a workaround for writing your own constexpr `construct_at` if you don't want to
// pull everything found in the <memory> header. Currently, this hack only works with Clang.
// But that's okay, since I'm not going to use the others.
namespace std::detail {

// clang-format off
template <typename T, typename... Args>
    requires requires(void* ptr, Args&&... args) { ::new (ptr) T(static_cast<Args&&>(args)...); }
    // ^^^ allows copy elision since C++17 mandates it
constexpr T* construct_at(T* const location, Args&&... args) noexcept {
    // @see: https://cplusplus.github.io/LWG/issue3870
    return ::new (static_cast<void*>(location)) T(::std::forward<Args>(args)...);
}
// clang-format on

} // namespace std::detail

namespace salt::fdn {

// clang-format off
using ::std::detail::construct_at;

template <typename ForwardIterator, typename T>
constexpr void construct_in_place(ForwardIterator position, T const& value) noexcept {
    construct_at(addressof(*position), value);
}
template <typename ForwardIterator, typename T>
constexpr void construct_in_place(ForwardIterator position, T&& value) noexcept {
    construct_at(addressof(*position), std::move(value));
}
template <typename ForwardIterator, typename... Args>
constexpr void construct_in_place(ForwardIterator position, Args&&... args) noexcept {
    construct_at(addressof(*position), std::forward<Args>(args)...);
}

template <typename ForwardIterator>
constexpr ForwardIterator destroy(ForwardIterator, ForwardIterator) noexcept;

template <typename T>
constexpr void destroy_at(T* const location) noexcept {
    location->~T();
}

template <typename ForwardIterator>
constexpr ForwardIterator destroy(ForwardIterator first, ForwardIterator last) noexcept {
    for (; first != last; ++first) {
        destroy_at(addressof(*first));
    }
    return first;
}

template <typename InputIterator>
constexpr void destroy_in_place(InputIterator position) noexcept {
    using value_type = std::iter_value_t<InputIterator>;
    if constexpr (meta::not_trivially_destructible<value_type>) {
        destroy_at(addressof(*position));
    }
}

template <typename T>
struct [[nodiscard]] destroy_guard final {
    T* value;
    constexpr explicit destroy_guard(T* const other) noexcept : value{other} {}
    constexpr ~destroy_guard() noexcept requires meta::trivially_destructible<T> = default;
    constexpr ~destroy_guard() noexcept { destroy_at(value); }
};

template <typename T, typename U>
constexpr U* relocate_at(T* const src, U* const dest) noexcept {
    if constexpr (meta::same_trivially_relocatable<T, U>) {
        if consteval {
            destroy_guard<T> guard{src};
            return construct_at(dest, std::move(*src));
        } else {
            detail::constexpr_memmove(dest, src);
            return launder(dest); // required?
        }
    } else {
        destroy_guard<T> guard{src};
        return construct_at(dest, std::move(*src));
    }
}
// clang-format on

} // namespace salt::fdn
