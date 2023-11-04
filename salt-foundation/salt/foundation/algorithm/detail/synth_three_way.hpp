#pragma once
#include <salt/meta.hpp>

#include <compare>

namespace salt::algorithm::detail {

template <typename T, typename U>
constexpr auto synth_three_way(T const& t, U const& u) noexcept
    requires requires {
        { t < u } -> meta::boolean_testable;
        { u < t } -> meta::boolean_testable;
    }
{
    if constexpr (std::three_way_comparable_with<T, U>) {
        return t <=> u;
    } else {
        // clang-format off
        if (t < u) return std::weak_ordering::less;
        if (u < t) return std::weak_ordering::greater;
        // clang-format on
        return std::weak_ordering::equivalent;
    }
}

template <typename T, typename U = T>
using synth_three_way_result = decltype(synth_three_way(meta::declval<T&>(), meta::declval<U&>()));

} // namespace salt::algorithm::detail