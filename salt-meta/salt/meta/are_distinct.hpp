#pragma once
#include <type_traits>

namespace salt {

template <typename T, typename... Ts>
struct [[nodiscard]] are_distinct
        : std::conjunction<std::negation<std::is_same<T, Ts>>..., are_distinct<Ts...>> {};

template <typename T> struct [[nodiscard]] are_distinct<T> : std::true_type {};

template <typename... Ts> static constexpr inline bool are_distinct_v = are_distinct<Ts...>::value;

template <typename... Ts>
concept distinct = are_distinct_v<Ts...>;

} // namespace salt