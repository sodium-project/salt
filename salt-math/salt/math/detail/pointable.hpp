#pragma once

namespace salt::detail {

template <typename T>
concept pointable = requires(T t) {
    t.x;
    t.y;
    requires std::integral<decltype(t.x)>;
    requires std::same_as<decltype(t.x), decltype(t.y)>;
    // Requires that the T is composed by only `x` and `y` (assuming no weird alignment stuff)
    requires sizeof(t) == sizeof(t.x) * 2;
    // Requires that constructing a T from `x` and `y` is a valid expression
    T{t.x, t.y};
};

// clang-format off

// Addition
template <pointable Point>
constexpr auto operator+(std::int32_t const scalar, Point const& p) noexcept {
    return Point{scalar + p.x, scalar + p.y};
}

template <pointable Point>
constexpr auto operator+(Point const& p, std::int32_t const scalar) noexcept {
    return Point{p.x + scalar, p.y + scalar};
}

template <pointable Point>
constexpr auto operator+(Point const& p0, Point const& p1) noexcept {
    return Point{p0.x + p1.x, p0.y + p1.y};
}

// Subtracting
template <pointable Point>
constexpr auto operator-(std::int32_t const scalar, Point const& p) noexcept {
    return Point{scalar - p.x, scalar - p.y};
}

template <pointable Point>
constexpr auto operator-(Point const& p, std::int32_t const scalar) noexcept {
    return Point{p.x - scalar, p.y - scalar};
}

template <pointable Point>
constexpr auto operator-(Point const& p0, Point const& p1) noexcept {
    return Point{p0.x - p1.x, p0.y - p1.y};
}

// Multiplication
template <pointable Point>
constexpr auto operator*(std::int32_t const scalar, Point const& p) noexcept {
    return Point{scalar * p.x, scalar * p.y};
}

template <pointable Point>
constexpr auto operator*(Point const& p, std::int32_t const scalar) noexcept {
    return Point{p.x * scalar, p.y * scalar};
}

template <pointable Point>
constexpr auto operator*(Point const& p0, Point const& p1) noexcept {
    return Point{p0.x * p1.x, p0.y * p1.y};
}

// Division
template <pointable Point>
constexpr auto operator/(std::int32_t const scalar, Point const& p) noexcept {
    return Point{scalar / p.x, scalar / p.y};
}

template <pointable Point>
constexpr auto operator/(Point const& p, std::int32_t const scalar) noexcept {
    return Point{p.x / scalar, p.y / scalar};
}

template <pointable Point>
constexpr auto operator/(Point const& p0, Point const& p1) noexcept {
    return Point{p0.x / p1.x, p0.y / p1.y};
}

// clang-format on

} // namespace salt::detail

namespace salt {

using salt::detail::operator+;
using salt::detail::operator-;
using salt::detail::operator*;
using salt::detail::operator/;

} // namespace salt