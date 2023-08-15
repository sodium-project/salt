#pragma once
#include <salt/config.hpp>
#include <salt/foundation/detail/constexpr_memcpy.hpp>
#include <salt/foundation/detail/synth_three_way.hpp>

namespace salt::fdn {

using std::exchange;

// clang-format off
template <typename T, typename U> requires meta::trivially_equality_comparable<T, U>
constexpr bool equal(T* first1, T* last1, U* first2) noexcept {
    return detail::constexpr_memcmp_equal(first1, first2, std::size_t(last1 - first1));
}

template <typename InputIterator1, typename InputIterator2>
constexpr bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2) noexcept {
    for (; first1 != last1; ++first1, ++first2) {
        if (!(*first1 == *first2))
            return false;
    }
    return true;
}

template <typename T>
constexpr T const& min(T const& a, T const& b) noexcept {
    return (b < a) ? b : a;
}
template <typename T>
constexpr T const& max(T const& a, T const& b) noexcept {
    return (a < b) ? b : a;
}
// clang-format on

namespace detail {

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

} // namespace detail

// clang-format off
template <
    meta::copy_constructible InputIterator1,
    meta::copy_constructible InputIterator2,
    typename                 Cmp
>
constexpr auto lexicographical_compare_three_way(InputIterator1 first1, InputIterator1 last1,
                                                 InputIterator2 first2, InputIterator2 last2,
                                                 Cmp& cmp) noexcept
        -> decltype(cmp(*first1, *first2)) {

    if constexpr (meta::has_random_access_iterator_category<InputIterator1> &&
                  meta::has_random_access_iterator_category<InputIterator2>) {
        return detail::lexicographical_compare_three_way_fast(
                meta::move(first1), meta::move(last1), meta::move(first2), meta::move(last2), cmp);
    } else {
        // Unoptimized implementation which compares the iterators against the end in every loop
        // iteration.
        return detail::lexicographical_compare_three_way_slow(
                meta::move(first1), meta::move(last1), meta::move(first2), meta::move(last2), cmp);
    }
}

template <typename InputIterator1, typename InputIterator2, typename Cmp>
constexpr auto lexicographical_compare_three_way(InputIterator1 first1, InputIterator1 last1,
                                                 InputIterator2 first2, InputIterator2 last2) noexcept {
    return lexicographical_compare_three_way(
                meta::move(first1), meta::move(last1),
                meta::move(first2), meta::move(last2), std::compare_three_way{});
}
// clang-format on

} // namespace salt::fdn
