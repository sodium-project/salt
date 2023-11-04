#pragma once
#include <salt/meta.hpp>
#include <salt/foundation/algorithm/minmax.hpp>

#include <compare>

namespace salt::algorithm::detail {

template <typename InputIterator1, typename InputIterator2, typename Cmp>
constexpr auto lexicographical_compare_three_way_fast(InputIterator1 first1, InputIterator1 last1,
                                                      InputIterator2 first2, InputIterator2 last2,
                                                      Cmp& cmp) noexcept
        -> decltype(cmp(*first1, *first2)) {

    using iter1_diff_t  = decltype(last1 - first1);
    using iter2_diff_t  = decltype(last2 - first2);
    using common_diff_t = meta::common_type<iter1_diff_t, iter2_diff_t>;

    iter1_diff_t  len1    = last1 - first1;
    iter2_diff_t  len2    = last2 - first2;
    common_diff_t min_len = min<common_diff_t>(len1, len2);

    for (common_diff_t i = 0; i < min_len; ++i, ++first1, ++first2) {
        auto result = cmp(*first1, *first2);
        if (result != 0)
            return result;
    }

    return len1 <=> len2;
}

template <typename InputIterator1, typename InputIterator2, typename Cmp>
constexpr auto lexicographical_compare_three_way_slow(InputIterator1 first1, InputIterator1 last1,
                                                      InputIterator2 first2, InputIterator2 last2,
                                                      Cmp& cmp) noexcept
        -> decltype(cmp(*first1, *first2)) {

    while (true) {
        bool exhausted1 = first1 == last1;
        bool exhausted2 = first2 == last2;

        if (exhausted1 || exhausted2) {
            // clang-format off
            if (!exhausted1) return std::strong_ordering::greater;
            if (!exhausted2) return std::strong_ordering::less;
            // clang-format on
            return std::strong_ordering::equal;
        }

        auto result = cmp(*first1, *first2);
        if (result != 0)
            return result;

        ++first1;
        ++first2;
    }
}

} // namespace salt::algorithm::detail