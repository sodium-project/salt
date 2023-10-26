#pragma once

namespace salt::meta {

using std::constructible_from;
using std::convertible_to;
using std::integral;
using std::same_as;
using std::movable;
using std::common_reference_with;
using std::derived_from;
using std::default_initializable;

template <typename T, std::size_t Size>
concept same_size = requires { requires sizeof(T) == Size; };

template <typename T, std::size_t Alignment>
concept same_alignment = requires { requires alignof(T) == Alignment; };

template <typename... Ts>
concept distinct = are_distinct_v<Ts...>;

template <typename T, typename... Ts>
concept contains = is_contains_v<T, Ts...>;

template <typename T>
concept trivial = std::is_trivial_v<T>;

template <typename T>
concept standard_layout = std::is_standard_layout_v<T>;

template <typename T>
concept default_constructible = std::is_default_constructible_v<T>;

template <typename T>
concept trivially_constructible = std::is_trivially_constructible_v<T>;

template <typename T>
concept trivially_default_constructible = std::is_trivially_default_constructible_v<T>;

template <class T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

template <typename T>
concept trivially_copy_constructible = std::is_trivially_copy_constructible_v<T>;
template <typename T>
concept trivially_move_constructible = std::is_trivially_move_constructible_v<T>;

template <typename T>
concept copy_assignable = std::is_copy_assignable_v<T>;
template <typename T>
concept move_assignable = std::is_move_assignable_v<T>;

template <typename T>
concept trivially_copy_assignable = std::is_trivially_copy_assignable_v<T>;
template <typename T>
concept trivially_move_assignable = std::is_trivially_move_assignable_v<T>;

template <typename T>
concept trivially_destructible = std::is_trivially_destructible_v<T>;
template <typename T>
concept not_trivially_destructible = not trivially_destructible<T>;
template <typename T>
concept destructible = std::is_destructible_v<T>;

template <typename T>
concept has_trivial_lifetime = trivially_default_constructible<T> and trivially_destructible<T>;

template <typename T>
concept nothrow_copy_constructible = std::is_nothrow_copy_constructible_v<T>;
template <typename T>
concept nothrow_move_constructible = std::is_nothrow_move_constructible_v<T>;

// clang-format off
// template <typename T>
// concept copy_constructible =
//     move_constructible<T>           and
//     constructible_from<T, T&      > and convertible_to<T&      , T> and
//     constructible_from<T, T const&> and convertible_to<T const&, T> and
//     constructible_from<T, T const > and convertible_to<T const , T>;
//
// template <typename T>
// concept move_constructible = constructible_from<T, T> and convertible_to<T, T>;
// clang-format on

template <typename T>
concept copy_constructible = std::copy_constructible<T>;
template <typename T>
concept move_constructible = std::move_constructible<T>;

template <typename T, typename U>
concept trivially_lexicographically_comparable =
        same_as<T, U> && sizeof(T) == 1 && std::is_unsigned_v<T>;

template <typename T, typename U>
concept trivially_equality_comparable = is_trivially_equality_comparable_v<T, U>;

namespace detail {
template <typename T>
concept boolean_testable_impl = convertible_to<T, bool>;
} // namespace detail

template <typename T>
concept boolean_testable = detail::boolean_testable_impl<T> and requires(T&& t) {
    { not forward<T>(t) } -> detail::boolean_testable_impl;
};

template <typename T>
concept object = std::is_object_v<T>;

template <typename T>
concept reference = std::is_reference_v<T>;
template <typename T>
concept not_reference = not reference<T>;
template <typename T>
concept pointer = std::is_pointer_v<T>;
template <typename T>
concept not_pointer = not pointer<T>;

template <typename T>
concept non_cv = same_as<std::remove_cv_t<T>, T>;

template <typename T>
concept integer = std::integral<T> and not same_as<remove_cvref_t<T>, bool>;
template <typename T>
concept unsigned_integer = std::unsigned_integral<T> and not same_as<remove_cvref_t<T>, bool>;

template <typename T, typename U = T>
concept not_volatile = std::is_volatile_v<T> and std::is_volatile_v<U>;

template <typename T, typename... Args>
concept underlying_constructible = std::conjunction_v<is_constructible_from<T, Args>...>;

} // namespace salt::meta