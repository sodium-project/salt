#pragma once
#include <salt/meta/type_traits.hpp>

namespace salt::meta {

namespace detail {

template <unsigned Size, typename... Ts>
struct [[nodiscard]] integral_size_list;

template <unsigned Size>
struct [[nodiscard]] integer_type;

template <unsigned Size>
struct [[nodiscard]] unsigned_integer_type;

} // namespace detail

template <unsigned Size, typename... Ts>
using integral_size_list_t = typename detail::integral_size_list<Size, Ts...>::type;

template <unsigned Size>
using integer_of_size_t = typename detail::integer_type<Size>::type;

template <unsigned Size>
using unsigned_integer_of_size_t = typename detail::unsigned_integer_type<Size>::type;

namespace detail {

template <unsigned Size, typename T, typename... Ts>
struct [[nodiscard]] integral_size_list<Size, T, Ts...> final {
    using type = condition<sizeof(T) == Size, T, integral_size_list_t<Size, Ts...>>;
};

template <unsigned Size>
struct [[nodiscard]] integral_size_list<Size> final {
    using type = void;
};

template <unsigned Size>
struct [[nodiscard]] integer_type final {
    using type = integral_size_list_t<Size, signed char, short, int, long, long long>;
};

template <unsigned Size>
struct [[nodiscard]] unsigned_integer_type final {
    using type = integral_size_list_t<Size, unsigned char, unsigned short, unsigned int,
                                      unsigned long, unsigned long long>;
};

} // namespace detail

} // namespace salt::meta