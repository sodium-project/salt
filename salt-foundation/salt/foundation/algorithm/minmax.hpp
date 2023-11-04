#pragma once

namespace salt::algorithm {

// clang-format off
template <typename T>
constexpr T const& min(T const& a, T const& b) noexcept {
    return (b < a) ? b : a;
}
template <typename T>
constexpr T const& max(T const& a, T const& b) noexcept {
    return (a < b) ? b : a;
}
// clang-format on

} // namespace salt::algorithm