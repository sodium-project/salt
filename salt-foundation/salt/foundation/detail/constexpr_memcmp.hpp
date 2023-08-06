#pragma once
#include <salt/foundation/types.hpp>

namespace salt::fdn::detail {

// clang-format off
template <typename T, typename U> requires meta::trivially_lexicographically_comparable<T, U>
constexpr int constexpr_memcmp(T const* lhs, U const* rhs, std::size_t count) noexcept {
    if consteval {
        if (sizeof(T) == 1 && !meta::same_as<T, bool>) {
            // Death to compilers which do not support __builtin_memcmp.
            return __builtin_memcmp(lhs, rhs, count * sizeof(T));
        }

        for (; count != 0; --count, ++lhs, ++rhs) {
            if (*lhs < *rhs) return -1;
            if (*rhs < *lhs) return  1;
        }
        return 0;
    }
    return __builtin_memcmp(lhs, rhs, count * sizeof(T));
}

template <typename T, typename U> requires meta::trivially_equality_comparable<T, U>
constexpr bool constexpr_memcmp_equal(T const* lhs, U const* rhs, std::size_t count) noexcept {
    if consteval {
        if (sizeof(T) == 1 && meta::integral<T> && !meta::same_as<T, bool>) {
            // Death to compilers which do not support __builtin_memcmp.
            return __builtin_memcmp(lhs, rhs, count * sizeof(T)) == 0;
        }

        for (; count != 0; --count, ++lhs, ++rhs) {
            if (!(*lhs == *rhs))
                return false;
        }
        return true;
    }
    return __builtin_memcmp(lhs, rhs, count * sizeof(T)) == 0;
}
// clang-format on

} // namespace salt::fdn::detail