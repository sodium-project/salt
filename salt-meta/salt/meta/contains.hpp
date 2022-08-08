#pragma once

namespace salt {

namespace detail {
template <typename T, typename... Ts>
struct [[nodiscard]] contains : std::disjunction<std::is_same<T, Ts>...> {};
} // namespace detail

template <typename T, typename... Ts>
static constexpr inline bool contains_v = detail::contains<T, Ts...>::value;

template <typename T, typename... Ts>
concept contains = contains_v<T, Ts...>;

} // namespace salt