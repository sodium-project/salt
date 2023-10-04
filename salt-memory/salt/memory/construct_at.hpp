#pragma once
#include <salt/memory/addressof.hpp>

#include <salt/memory/detail/constexpr_memcpy.hpp>

namespace salt::memory {

// clang-format off
template <typename T, typename... Args>
constexpr T* construct_at(T* pointer, Args&&... args) noexcept {
    return std::construct_at(pointer, meta::forward<Args>(args)...);
}

template <meta::not_pointer ForwardIterator, typename T>
constexpr void construct_at(ForwardIterator position, T const& value) noexcept {
    std::construct_at(addressof(*position), value);
}
template <meta::not_pointer ForwardIterator, typename T>
constexpr void construct_at(ForwardIterator position, T&& value) noexcept {
    std::construct_at(addressof(*position), meta::move(value));
}
template <meta::not_pointer ForwardIterator, typename... Args>
constexpr void construct_at(ForwardIterator position, Args&&... args) noexcept {
    std::construct_at(addressof(*position), meta::forward<Args>(args)...);
}

template <typename T>
constexpr void destroy_at(T* pointer) noexcept {
    std::destroy_at(pointer);
}

template <meta::not_pointer InputIterator>
constexpr void destroy_at(InputIterator position) noexcept {
    using value_type = meta::iter_value_t<InputIterator>;
    if constexpr (meta::not_trivially_destructible<value_type>) {
        std::destroy_at(addressof(*position));
    }
}

template <typename T>
struct [[nodiscard]] destroy_guard final {
    T* value;
    constexpr explicit destroy_guard(T* other) noexcept : value{other} {}
    constexpr ~destroy_guard() noexcept requires meta::trivially_destructible<T> = default;
    constexpr ~destroy_guard() noexcept { std::destroy_at(value); }
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

template <typename ForwardIterator>
constexpr void destroy(ForwardIterator first, ForwardIterator last) noexcept {
    using value_type = meta::iter_value_t<ForwardIterator>;
    for (; first != last; ++first) {
        destroy_guard<value_type> guard{addressof(*first)};
    }
}
// clang-format on

} // namespace salt::memory
