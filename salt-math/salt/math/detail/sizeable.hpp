#pragma once

namespace salt::detail {

template <typename T>
concept sizeable = requires(T t) {
    t.width;
    t.height;
    requires std::integral<decltype(t.width)>;
    requires std::same_as<decltype(t.width), decltype(t.height)>;
    // Requires that the T is composed by only `width` and `height` (assuming no weird alignment stuff)
    requires sizeof(t) == sizeof(t.width) * 2;
    // Requires that constructing a T from `width` and `height` is a valid expression
    T{t.width, t.height};
};

// clang-format off

// Addition
template <sizeable Size>
constexpr auto operator+(std::size_t const scalar, Size const& p) noexcept {
    return Size{scalar + p.width, scalar + p.height};
}

template <sizeable Size>
constexpr auto operator+(Size const& p, std::size_t const scalar) noexcept {
    return Size{p.width + scalar, p.height + scalar};
}

template <sizeable Size>
constexpr auto operator+(Size const& p0, Size const& p1) noexcept {
    return Size{p0.width + p1.width, p0.height + p1.height};
}

// Subtracting
template <sizeable Size>
constexpr auto operator-(std::size_t const scalar, Size const& p) noexcept {
    return Size{scalar - p.width, scalar - p.height};
}

template <sizeable Size>
constexpr auto operator-(Size const& p, std::size_t const scalar) noexcept {
    return Size{p.width - scalar, p.height - scalar};
}

template <sizeable Size>
constexpr auto operator-(Size const& p0, Size const& p1) noexcept {
    return Size{p0.width - p1.width, p0.height - p1.height};
}

// Multiplication
template <sizeable Size>
constexpr auto operator*(std::size_t const scalar, Size const& p) noexcept {
    return Size{scalar * p.width, scalar * p.height};
}

template <sizeable Size>
constexpr auto operator*(Size const& p, std::size_t const scalar) noexcept {
    return Size{p.width * scalar, p.height * scalar};
}

template <sizeable Size>
constexpr auto operator*(Size const& p0, Size const& p1) noexcept {
    return Size{p0.width * p1.width, p0.height * p1.height};
}

// Division
template <sizeable Size>
constexpr auto operator/(std::size_t const scalar, Size const& p) noexcept {
    return Size{scalar / p.width, scalar / p.height};
}

template <sizeable Size>
constexpr auto operator/(Size const& p, std::size_t const scalar) noexcept {
    return Size{p.width / scalar, p.height / scalar};
}

template <sizeable Size>
constexpr auto operator/(Size const& p0, Size const& p1) noexcept {
    return Size{p0.width / p1.width, p0.height / p1.height};
}

// clang-format on

} // namespace salt::detail

namespace salt {

using salt::detail::operator+;
using salt::detail::operator-;
using salt::detail::operator*;
using salt::detail::operator/;

} // namespace salt