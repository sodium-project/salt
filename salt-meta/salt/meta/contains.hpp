#pragma once

namespace salt::meta {

namespace detail {
template <typename T, typename... Ts>
struct [[nodiscard]] contains : std::disjunction<std::is_same<T, Ts>...> {};
} // namespace detail

template <typename T, typename... Ts>
inline constexpr bool contains_v = detail::contains<T, Ts...>::value;

template <typename T, typename... Ts>
concept contains = contains_v<T, Ts...>;

} // namespace salt::meta