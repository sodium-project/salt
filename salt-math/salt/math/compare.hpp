#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

#include <compare>

namespace salt {

template <typename Floating_point>
requires std::is_floating_point_v<Floating_point>
[[nodiscard]] static auto compare(Floating_point const lhs, Floating_point const rhs,
                                  Floating_point const epsilon) noexcept {
    auto const                   delta    = lhs - rhs;
    constexpr std::weak_ordering result[] = {
            /* 0 */ std::weak_ordering::equivalent,
            /* 1 */ std::weak_ordering::greater,
            /* 2 */ std::weak_ordering::equivalent,
            /* 3 */ std::weak_ordering::less,
    };
    //      a=signbit  b=abs(delta)>epsilon  (a<<1)|b
    // -1       1               1                3
    // -0       1               0                2
    //  0       0               0                0
    //  1       0               1                1
    auto const a = std::signbit(delta);
    auto const b = std::abs(delta) > epsilon;
    return result[(a << 1) | b];
}

template <int L1, int L2>
[[nodiscard]] static auto compare(glm::vec<L1, double> const& lhs, glm::vec<L2, double> const& rhs,
                                  double const epsilon) noexcept {
    for (auto i = std::min(L1, L2); --i;) {
        auto const order = compare(lhs[i], rhs[i], epsilon);
        if (order != std::weak_ordering::equivalent) {
            return order;
        }
    }
    return std::weak_ordering::equivalent;
}

[[nodiscard]] constexpr static auto compare(int const lhs, int const rhs) noexcept {
    auto const delta = lhs - rhs;
    return delta ? (delta < 0 ? std::strong_ordering::less : std::strong_ordering::greater)
                 : std::strong_ordering::equal;
}

} // namespace salt