#pragma once
#include <salt/foundation/algorithm/detail/lexicographical_compare_three_way.hpp>
#include <salt/foundation/algorithm/detail/synth_three_way.hpp>

namespace salt::algorithm {

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

} // namespace salt::algorithm