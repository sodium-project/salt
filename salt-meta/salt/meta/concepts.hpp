#pragma once

namespace salt::meta {

using std::constructible_from;
using std::convertible_to;
using std::integral;
using std::same_as;

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
concept trivially_copy_assignable = std::is_trivially_copy_assignable_v<T>;
template <typename T>
concept trivially_move_assignable = std::is_trivially_move_assignable_v<T>;

template <typename T>
concept trivially_destructible = std::is_trivially_destructible_v<T>;

template <typename T>
concept has_trivial_lifetime = trivially_default_constructible<T> and trivially_destructible<T>;

template <typename T>
concept nothrow_copy_constructible = std::is_nothrow_copy_constructible_v<T>;
template <typename T>
concept nothrow_move_constructible = std::is_nothrow_move_constructible_v<T>;

template <typename T>
concept copy_constructible = std::is_copy_constructible_v<T>;
template <typename T>
concept move_constructible = std::is_move_constructible_v<T>;

template <typename T, typename U>
concept trivially_lexicographically_comparable =
        same_as<T, U> && sizeof(T) == 1 && std::is_unsigned_v<T>;

template <typename T, typename U>
concept trivially_equality_comparable = is_trivially_equality_comparable_v<T, U>;

template <typename T>
concept has_iterator_category = requires { typename T::iterator_category; };

template <typename T, typename U>
concept iterator_category_convertible_to =
        has_iterator_category<std::iter_value_t<T>> and
        convertible_to<typename std::iterator_traits<T>::iterator_category, U>;

template <typename T>
concept has_random_access_iterator_category =
        iterator_category_convertible_to<T, std::random_access_iterator_tag>;

namespace detail {
template <typename T>
concept boolean_testable_impl = convertible_to<T, bool>;
} // namespace detail

template <typename T>
concept boolean_testable = detail::boolean_testable_impl<T> and requires(T&& t) {
    { not std::forward<T>(t) } -> detail::boolean_testable_impl;
};

} // namespace salt::meta