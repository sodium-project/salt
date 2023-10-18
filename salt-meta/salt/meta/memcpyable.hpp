#pragma once

namespace salt::meta {

// clang-format off
namespace detail {

template <typename InputIterator, typename OutputIterator>
struct [[nodiscard]] is_memcpyable final {
    using T = iter_value_t<OutputIterator>;
    using U = decltype(iter_move(std::declval<InputIterator&&>()));

    static constexpr bool value = same_as<T, remove_ref_t<U>> and trivially_copyable<T>;
};

template <typename InputIterator, typename OutputIterator>
inline constexpr bool is_memcpyable_v = is_memcpyable<InputIterator, OutputIterator>::value;

template <typename T, typename U>
concept are_pointers = (pointer<T> and pointer<U>);

} // namespace detail

template <typename T, typename U>
concept contiguous_pointers  = detail::are_pointers<T, U> and same_as<remove_cvref_t<T>, remove_cvref_t<U>>;
template <typename T, typename U>
concept contiguous_iterators = contiguous_iterator<T> and contiguous_iterator<U>;
template <typename T, typename U = T>
concept contiguous = contiguous_pointers<T, U> or contiguous_iterators<T, U>;
template <typename T, typename U = T>
concept memcpyable = detail::is_memcpyable_v<T, U> and contiguous<T, U>;
// clang-format on

} // namespace salt::meta