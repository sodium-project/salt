#pragma once

namespace salt::meta {

template <typename T, typename... Ts>
struct [[nodiscard]] are_distinct
        : std::conjunction<std::negation<std::is_same<T, Ts>>..., are_distinct<Ts...>> {};

template <typename T>
struct [[nodiscard]] are_distinct<T> : std::true_type {};

template <typename... Ts>
inline constexpr bool are_distinct_v = are_distinct<Ts...>::value;

template <typename T, typename... Ts>
struct [[nodiscard]] is_contains : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename... Ts>
inline constexpr bool is_contains_v = is_contains<T, Ts...>::value;

namespace detail {

template <bool> struct [[nodiscard]] if_;

template <> struct [[nodiscard]] if_<true> final {
    template <typename T, typename F> using type = T;
};

template <> struct [[nodiscard]] if_<false> final {
    template <typename T, typename F> using type = F;
};

} // namespace detail

template <bool Condition, typename T, typename F>
using condition = typename detail::if_<Condition>::template type<T, F>;

template <typename T, template <typename...> typename Template>
struct [[nodiscard]] is_specialization : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct [[nodiscard]] is_specialization<Template<Args...>, Template> : std::true_type {};

template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;

template <typename T>
struct [[nodiscard]] remove_all_pointers
        : std::conditional_t<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>,
                             std::type_identity<T>> {};

template <typename T> using remove_all_pointers_t = typename remove_all_pointers<T>::type;

} // namespace salt::meta