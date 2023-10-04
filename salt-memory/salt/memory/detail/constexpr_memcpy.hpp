#pragma once
#include <cstddef>
#include <salt/meta.hpp>

namespace salt::memory::detail {

// clang-format off
template <typename T, typename U> requires meta::trivially_lexicographically_comparable<T, U>
constexpr int constexpr_memcmp(T const* lhs, U const* rhs, std::size_t count) noexcept {
    if consteval {
        if (!meta::same_as<T, bool>) {
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
        if (sizeof(T) == 1 && !meta::same_as<T, bool>) {
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

template <typename T, typename U>
constexpr auto constexpr_memcpy(T* dest, U const* src, std::size_t count = 1) noexcept {
    if consteval {
        if (meta::same_as<meta::remove_cv_t<T>, meta::remove_cv_t<U>>) {
            // Death to compilers which do not support __builtin_memcpy.
           return __builtin_memcpy(dest, src, count * sizeof(T));
        }
    }
    return __builtin_memcpy(dest, src, count * sizeof(T));
}

template <typename T, typename U>
constexpr auto constexpr_memmove(T* dest, U const* src, std::size_t count = 1) noexcept {
    if consteval {
        if (meta::same_as<meta::remove_cv_t<T>, meta::remove_cv_t<U>>) {
            // Death to compilers which do not support __builtin_memmove.
           return __builtin_memmove(dest, src, count * sizeof(T));
        }
    }
    return __builtin_memmove(dest, src, count * sizeof(T));
}
// clang-format on

} // namespace salt::memory::detail