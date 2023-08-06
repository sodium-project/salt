#pragma once

namespace salt::meta {

// clang-format off
template <typename T, typename... Ts>
struct [[nodiscard]] are_distinct
        : std::conjunction<std::negation<std::is_same<T, Ts>>..., are_distinct<Ts...>> {};

template <typename T> struct [[nodiscard]] are_distinct<T> : std::true_type {};

template <typename... Ts>
inline constexpr bool are_distinct_v = are_distinct<Ts...>::value;

template <typename T, typename... Ts>
struct [[nodiscard]] is_contains : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename... Ts>
inline constexpr bool is_contains_v = is_contains<T, Ts...>::value;
// clang-format on

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

template <typename... T> using common_type = std::common_type_t<T...>;

template <typename T, template <typename...> typename Template>
struct [[nodiscard]] is_specialization : std::false_type {};

template <template <typename...> typename Template, typename... Args>
struct [[nodiscard]] is_specialization<Template<Args...>, Template> : std::true_type {};

template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;

template <typename T>
struct [[nodiscard]] remove_all_pointers
        : condition<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>,
                    std::type_identity<T>> {};

// clang-format off
template <typename T>
using remove_all_pointers_t = typename remove_all_pointers<T>::type;
// clang-format on

template <typename T, typename U, typename = void>
struct [[nodiscard]] is_equality_comparable : std::false_type {};

template <typename T, typename U>
struct [[nodiscard]] is_equality_comparable<
        T, U, std::void_t<decltype(std::declval<T>() == std::declval<U>())>> : std::true_type {};

template <typename T, typename U>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;

// clang-format off
template <typename T, typename U>
struct [[nodiscard]] is_trivially_equality_comparable_impl : std::false_type {};
// clang-format on

template <typename T>
struct [[nodiscard]] is_trivially_equality_comparable_impl<T, T>
#if __has_builtin(__is_trivially_equality_comparable)
        : std::bool_constant<__is_trivially_equality_comparable(T) &&
                             is_equality_comparable_v<T, T>>
#else
        : std::is_integral<T>
#endif
{
};

template <typename T>
struct [[nodiscard]] is_trivially_equality_comparable_impl<T*, T*> : std::true_type {};

template <typename T, typename U>
struct [[nodiscard]] is_trivially_equality_comparable_impl<T*, U*>
        : std::bool_constant<is_equality_comparable_v<T*, U*> &&
                             (std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>> ||
                              std::is_void_v<T> || std::is_void_v<U>)> {};

template <typename T, typename U>
using is_trivially_equality_comparable =
        is_trivially_equality_comparable_impl<std::remove_cv_t<T>, std::remove_cv_t<U>>;

template <typename T, typename U>
inline constexpr bool is_trivially_equality_comparable_v =
        is_trivially_equality_comparable<T, U>::value;

template <typename First, typename... Rest>
    requires std::conjunction_v<std::is_same<First, Rest>...>
struct [[nodiscard]] enforce_same {
    using type = First;
};

template <typename First, typename... Rest>
using enforce_same_t = typename enforce_same<First, Rest...>::type;

} // namespace salt::meta