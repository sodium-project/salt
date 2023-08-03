#pragma once

namespace salt::meta {

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
using condition = typename details::if_<Condition>::template type<T, F>;

} // namespace salt::meta