#pragma once

namespace salt::meta {

using std::constructible_from;
using std::convertible_to;
using std::integral;
using std::input_iterator;
using std::forward_iterator;
using std::random_access_iterator;
using std::contiguous_iterator;
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

template <typename T>
concept trivially_relocatable = trivially_copyable<T>; // trivially_move_constructible<T> and trivially_destructible<T>;
template <typename T>
concept relocatable = trivially_copyable<T>; // move_constructible<T>;

// clang-format off
template <typename T, typename U = T>
concept contiguous = (pointer<T>             and pointer<U>) or
                     (contiguous_iterator<T> and contiguous_iterator<U>);

namespace detail {
template <typename InputIterator>
constexpr decltype(auto) iter_move(InputIterator&& it) noexcept {
    return meta::move(*it);
}
template <typename InputIterator, typename OutputIterator>
struct [[nodiscard]] is_memcpyable final {
    using T = iter_value_t<OutputIterator>;
    using U = decltype(iter_move(std::declval<InputIterator>()));

    static constexpr bool value = same_as<T, remove_ref_t<U>> and trivially_copyable<T>;
};
template <typename InputIterator, typename OutputIterator>
inline constexpr bool is_memcpyable_v = is_memcpyable<InputIterator, OutputIterator>::value;
} // namespace detail

template <typename T, typename U>
concept memcpyable = detail::is_memcpyable_v<T, U> and contiguous<T, U>;
// clang-format on

template <typename T, typename U = T>
concept not_volatile = std::is_volatile_v<T> and std::is_volatile_v<U>;

template <typename T, typename U>
concept same_trivially_relocatable =
        is_same_uncvref_v<T, U> and trivially_relocatable<remove_cvref_t<U>> and not_volatile<T, U>;

template <typename T, typename... Args>
concept underlying_constructible = std::conjunction_v<is_constructible_from<T, Args>...>;

} // namespace salt::meta