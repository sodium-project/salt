#pragma once

namespace salt {

template <typename T, typename... Ts>
struct [[nodiscard]] contains : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename... Ts>
static constexpr inline bool contains_v = contains<T, Ts...>::value;

} // namespace salt