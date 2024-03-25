#pragma once

namespace salt::meta {

// clang-format off
template <typename T>
using remove_ref_t = std::remove_reference_t<T>;
template <typename... T>
using common_type = std::common_type_t<T...>;

using std::bool_constant;
using std::true_type;
using std::false_type;
using std::decay_t;
using std::remove_cv_t;
using std::remove_cvref_t;
using std::remove_all_extents_t;
using std::add_pointer_t;
using std::void_t;
using std::tuple;
using std::apply;
using std::random_access_iterator_tag;
using std::contiguous_iterator_tag;

template <typename T>
using iter_diff_t = std::iter_difference_t<T>;

template <typename T>
inline constexpr bool is_class_v = std::is_class_v<T>;
template <typename T>
inline constexpr bool is_empty_v = std::is_empty_v<T>;

template <typename T>
inline constexpr bool is_lvalue_reference_v = std::is_lvalue_reference_v<T>;

template <typename T, typename... Ts>
struct [[nodiscard]] are_distinct
        : std::conjunction<std::negation<std::is_same<T, Ts>>..., are_distinct<Ts...>> {};

template <typename T> struct [[nodiscard]] are_distinct<T> : true_type {};

template <typename... Ts>
inline constexpr bool are_distinct_v = are_distinct<Ts...>::value;

template <typename T, typename... Ts>
struct [[nodiscard]] is_contains : std::disjunction<std::is_same<T, Ts>...> {};

template <typename T, typename... Ts>
inline constexpr bool is_contains_v = is_contains<T, Ts...>::value;
// clang-format on

namespace detail {

template <bool>
struct [[nodiscard]] if_;

template <>
struct [[nodiscard]] if_<true> final {
    template <typename T, typename F>
    using type = T;
};

template <>
struct [[nodiscard]] if_<false> final {
    template <typename T, typename F>
    using type = F;
};

} // namespace detail

template <bool Condition, typename T, typename F>
using condition = typename detail::if_<Condition>::template type<T, F>;

template <typename T, template <typename...> typename Template>
struct [[nodiscard]] is_specialization : false_type {};

template <template <typename...> typename Template, typename... Args>
struct [[nodiscard]] is_specialization<Template<Args...>, Template> : true_type {};

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
struct [[nodiscard]] is_equality_comparable : false_type {};

template <typename T, typename U>
struct [[nodiscard]] is_equality_comparable<
        T, U, std::void_t<decltype(declval<T>() == declval<U>())>> : true_type {};

template <typename T, typename U>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;

// clang-format off
template <typename T, typename U>
struct [[nodiscard]] is_trivially_equality_comparable_impl : false_type {};
// clang-format on

template <typename T>
struct [[nodiscard]] is_trivially_equality_comparable_impl<T, T>
#if __has_builtin(__is_trivially_equality_comparable)
        : std::bool_constant<__is_trivially_equality_comparable(T) and
                             is_equality_comparable_v<T, T>>
#else
        : std::is_integral<T>
#endif
{
};

template <typename T>
struct [[nodiscard]] is_trivially_equality_comparable_impl<T*, T*> : true_type {};

// clang-format off
template <typename T, typename U>
struct [[nodiscard]] is_trivially_equality_comparable_impl<T*, U*>
        : std::bool_constant<is_equality_comparable_v<T*, U*> and
                             (std::is_same_v<remove_cv_t<T>, remove_cv_t<U>> or
                              std::is_void_v<T> or std::is_void_v<U>)> {};
// clang-format on

template <typename T, typename U>
using is_trivially_equality_comparable =
        is_trivially_equality_comparable_impl<remove_cv_t<T>, remove_cv_t<U>>;

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

template <typename F> struct [[nodiscard]] deduce;

// clang-format off
template <typename F, typename... Args>
struct [[nodiscard]] deduce<F(Args...)> {
    using type = std::invoke_result_t<F, Args...>;
};
template <typename T>
using deduce_t = typename deduce<T>::type;

template <typename T>
struct [[nodiscard]] deref {
    using type = decltype(*declval<T>());
};
template <typename T>
using deref_t = typename deref<T>::type;

template <typename T, typename U>
using is_same_as = std::bool_constant<__is_same(T, U)>;

template <typename T, typename U>
struct [[nodiscard]] is_same_uncvref : is_same_as<remove_cvref_t<T>, remove_cvref_t<U>> {};
template <typename T, typename U>
inline constexpr bool is_same_uncvref_v = is_same_uncvref<T, U>::value;
// clang-format on

template <typename T, typename U>
struct [[nodiscard]] is_constructible_from
        : std::bool_constant<std::is_nothrow_destructible_v<T> && std::is_constructible_v<T, U>> {
};

template <typename T>
struct [[nodiscard]] template_parameter;

template <template <typename...> class C, typename T>
struct [[nodiscard]] template_parameter<C<T>> final {
    using type = T;
};

template <typename T>
using template_parameter_t = typename template_parameter<T>::type;

} // namespace salt::meta