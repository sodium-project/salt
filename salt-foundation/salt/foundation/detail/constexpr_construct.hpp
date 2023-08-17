#pragma once
#include <salt/foundation/addressof.hpp>
#include <salt/foundation/types.hpp>

#include <salt/foundation/detail/constexpr_memcpy.hpp>

namespace salt::fdn::detail {

// clang-format off
template <typename T>
struct [[nodiscard]] destroy_guard final {
    T* value;
    constexpr explicit destroy_guard(T* other) noexcept : value{other} {}
    constexpr ~destroy_guard() requires meta::trivially_destructible<T> = default;
    constexpr ~destroy_guard() { std::destroy_at(value); }
};

template <typename T, typename U>
constexpr U* relocate_at(T* src, U* dest) noexcept {
    if constexpr (meta::same_trivially_relocatable<T, U>) {
        if consteval {
            destroy_guard<T> guard{src};
            return std::construct_at(dest, meta::move(*src));
        } else {
            detail::constexpr_memmove(dest, src);
            return launder(dest); // required?
        }
    } else {
        destroy_guard<T> guard{src};
        return std::construct_at(dest, meta::move(*src));
    }
}

template <typename ForwardIterator, typename T>
constexpr void construct_at(ForwardIterator position, T const& value) noexcept {
    std::construct_at(addressof(*position), value);
}
template <typename ForwardIterator, typename T>
constexpr void construct_at(ForwardIterator position, T&& value) noexcept {
    std::construct_at(addressof(*position), meta::move(value));
}
template <typename ForwardIterator, typename... Args>
constexpr void construct_at(ForwardIterator position, Args&&... args) noexcept {
    std::construct_at(addressof(*position), meta::forward<Args>(args)...);
}

template <typename InputIterator>
constexpr void destroy_at(InputIterator position) noexcept {
    using value_type = meta::iter_value_t<InputIterator>;
    if constexpr (meta::not_trivially_destructible<value_type>) {
        std::destroy_at(addressof(*position));
    }
}

template <typename ForwardIterator>
constexpr void destroy(ForwardIterator first, ForwardIterator last) noexcept {
    using value_type = meta::iter_value_t<ForwardIterator>;
    for (; first != last; ++first) {
        destroy_guard<value_type> guard{addressof(*first)};
    }
}
// clang-format on

} // namespace salt::fdn::detail
