#pragma once

#include <salt/meta.hpp>

namespace salt::cxx20 {

#if SALT_TARGET_WINDOWS

using std::ranges::borrowed_range;
using std::ranges::distance;
using std::ranges::random_access_range;
using std::ranges::is_permutation

// Since Apple put a big willie on the implementation of features from C++20, I have to implement
// everything instead of them. What could be better than that?
#elif SALT_TARGET_MACOSX && SALT_CLANG_FULL_VER <= 130106
template <typename T>
concept range = requires(T& t) {
    std::ranges::begin(t);
    std::ranges::end(t);
};

template <typename R>
concept borrowed_range = range<R> &&(std::is_lvalue_reference_v<R> or
                                     std::ranges::enable_borrowed_range<std::remove_cvref_t<R>>);

template <typename T>
concept input_range = range<T> && std::input_iterator<std::ranges::iterator_t<T>>;

template <typename T>
concept forward_range = input_range<T> && std::forward_iterator<std::ranges::iterator_t<T>>;

template <typename T>
concept bidirectional_range =
        forward_range<T> && std::bidirectional_iterator<std::ranges::iterator_t<T>>;

template <typename T>
concept random_access_range =
        bidirectional_range<T> && std::random_access_iterator<std::ranges::iterator_t<T>>;

template <typename I0, typename I1, typename Comp, typename Proj0 = std::identity,
          typename Proj1 = std::identity>
concept indirectly_comparable =
        std::indirect_binary_predicate<Comp, std::projected<I0, Proj0>, std::projected<I1, Proj1>>;

// clang-format off
template <std::input_or_output_iterator I, std::sentinel_for<I> S> requires(!std::sized_sentinel_for<S, I>)
constexpr std::iter_difference_t<I> distance(I first, S last) {
    std::iter_difference_t<I> result = 0;
    while (first != last) {
        ++first;
        ++result;
    }
    return result;
}
// clang-format on

template <std::input_or_output_iterator I, std::sized_sentinel_for<I> S>
constexpr std::iter_difference_t<I> distance(I const& first, S const& last) {
    return last - first;
}

template <range R> using range_difference_t = std::iter_difference_t<std::ranges::iterator_t<R>>;

template <typename I1, typename I2> struct in_in_result {
    [[no_unique_address]] I1 in1;
    [[no_unique_address]] I2 in2;

    template <typename II1, typename II2>
    requires std::convertible_to<I1 const&, II1> && std::convertible_to<I2 const&, II2>
    constexpr operator in_in_result<II1, II2>() const& {
        return {in1, in2};
    }

    template <typename II1, typename II2>
    requires std::convertible_to<I1, II1> && std::convertible_to<I2, II2>
    constexpr operator in_in_result<II1, II2>() && {
        return {std::move(in1), std::move(in2)};
    }
};

template <typename I1, typename I2> using mismatch_result = in_in_result<I1, I2>;

struct dangling {
    dangling() = default;
    constexpr dangling(auto&&...) noexcept {}
};

template <range R>
using borrowed_iterator_t =
        std::conditional_t<borrowed_range<R>, std::ranges::iterator_t<R>, dangling>;

// clang-format off
template <std::input_iterator I1, std::sentinel_for<I1> S1,
          std::input_iterator I2, std::sentinel_for<I2> S2,
          typename Pred  = std::ranges::equal_to,
          typename Proj1 = std::identity, class Proj2 = std::identity>
requires indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
// clang-format on
constexpr mismatch_result<I1, I2> mismatch(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
                                           Proj1 proj1 = {}, Proj2 proj2 = {}) {
    for (; first1 != last1 && first2 != last2; ++first1, (void)++first2) {
        if (!std::invoke(pred, std::invoke(proj1, *first1), std::invoke(proj2, *first2))) {
            break;
        }
    }
    return {first1, first2};
}

// clang-format off
template <input_range R1, input_range R2,
          typename Pred = std::ranges::equal_to,
          typename Proj1 = std::identity, typename Proj2 = std::identity>
requires indirectly_comparable<
               std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>, Pred, Proj1, Proj2>
// clang-format on
constexpr mismatch_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>>
mismatch(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) {
    return mismatch(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2),
                    std::ranges::end(r2), std::ref(pred), std::ref(proj1), std::ref(proj2));
}

// clang-format off
template <std::input_iterator I, std::sentinel_for<I> S,
          typename Proj = std::identity,
          std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
// clang-format on
constexpr std::iter_difference_t<I> count_if(I first, S last, Pred pred, Proj proj = {}) {
    std::iter_difference_t<I> counter = 0;
    for (; first != last; ++first) {
        if (std::invoke(pred, std::invoke(proj, *first))) {
            ++counter;
        }
    }
    return counter;
}

template <input_range R, typename Proj = std::identity,
          std::indirect_unary_predicate<std::projected<std::ranges::iterator_t<R>, Proj>> Pred>
constexpr range_difference_t<R> count_if(R&& r, Pred pred, Proj proj = {}) {
    return count_if(std::ranges::begin(r), std::ranges::end(r), std::ref(pred), std::ref(proj));
}

template <std::input_iterator I, std::sentinel_for<I> S, typename Proj = std::identity,
          std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
constexpr I find_if(I first, S last, Pred pred, Proj proj = {}) {
    for (; first != last; ++first) {
        if (std::invoke(pred, std::invoke(proj, *first))) {
            return first;
        }
    }
    return first;
}

template <input_range R, typename Proj = std::identity,
          std::indirect_unary_predicate<std::projected<std::ranges::iterator_t<R>, Proj>> Pred>
constexpr borrowed_iterator_t<R> find_if(R&& r, Pred pred, Proj proj = {}) {
    return find_if(std::ranges::begin(r), std::ranges::end(r), std::ref(pred), std::ref(proj));
}

// clang-format off
template <std::forward_iterator I1, std::sentinel_for<I1> S1,
          std::forward_iterator I2, std::sentinel_for<I2> S2,
          typename Proj1 = std::identity, typename Proj2 = std::identity,
          std::indirect_equivalence_relation<std::projected<I1, Proj1>,
                                             std::projected<I2, Proj2>>
                                                Pred = std::ranges::equal_to>
// clang-format on
constexpr bool is_permutation(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {},
                              Proj1 proj1 = {}, Proj2 proj2 = {}) {
    // skip common prefix
    auto ret = mismatch(first1, last1, first2, last2, std::ref(pred), std::ref(proj1),
                        std::ref(proj2));
    first1 = ret.in1, first2 = ret.in2;

    // iterate over the rest, counting how many times each element
    // from [first1, last1) appears in [first2, last2)
    for (auto i{first1}; i != last1; ++i) {
        const auto i_proj{std::invoke(proj1, *i)};
        auto       i_cmp = [&]<typename T>(T&& t) {
            return std::invoke(pred, i_proj, std::forward<T>(t));
        };

        if (i != find_if(first1, i, i_cmp, proj1))
            continue; // this *i has been checked

        const auto m{count_if(first2, last2, i_cmp, proj2)};
        if (m == 0 or m != count_if(i, last1, i_cmp, proj1))
            return false;
    }
    return true;
}

// clang-format off
template <forward_range R1, forward_range R2,
          typename Proj1 = std::identity, typename Proj2 = std::identity,
          std::indirect_equivalence_relation<std::projected<std::ranges::iterator_t<R1>, Proj1>,
                                             std::projected<std::ranges::iterator_t<R2>, Proj2>>
                                                Pred = std::ranges::equal_to>
// clang-format on
constexpr bool is_permutation(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {},
                              Proj2 proj2 = {}) {
    return is_permutation(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2),
                          std::ranges::end(r2), std::move(pred), std::move(proj1),
                          std::move(proj2));
}
#endif

} // namespace salt::cxx20