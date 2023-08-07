#pragma once

namespace salt::meta {

// clang-format off
template <auto Lhs, auto Rhs>
inline constexpr bool equal = Lhs == Rhs;
template <auto Lhs, auto Rhs>
inline constexpr bool not_equal = Lhs != Rhs;

template <auto Lhs, auto Rhs>
inline constexpr bool less = Lhs < Rhs;
template <auto Lhs, auto Rhs>
inline constexpr bool less_or_equal = Lhs <= Rhs;

template <auto Lhs, auto Rhs>
inline constexpr bool greater = Lhs > Rhs;
template <auto Lhs, auto Rhs>
inline constexpr bool greater_or_equal = Lhs >= Rhs;
// clang-format on

} // namespace salt::meta