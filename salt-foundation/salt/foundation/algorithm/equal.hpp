#pragma once
#include <salt/foundation/memory/detail/constexpr_memcpy.hpp>

namespace salt::algorithm {

// clang-format off
template <typename T, typename U> requires meta::trivially_equality_comparable<T, U>
constexpr bool equal(T* first1, T* last1, U* first2) noexcept {
    return memory::detail::constexpr_memcmp_equal(first1, first2, std::size_t(last1 - first1));
}

template <typename InputIterator1, typename InputIterator2>
constexpr bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2) noexcept {
    for (; first1 != last1; ++first1, ++first2) {
        if (!(*first1 == *first2))
            return false;
    }
    return true;
}
// clang-format on

} // namespace salt::algorithm