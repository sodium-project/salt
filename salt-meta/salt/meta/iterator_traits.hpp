#pragma once
#include <cstddef>

namespace salt::meta {

namespace detail {

// clang-format off
template <typename T>
using with_reference = T&;

template <typename T>
concept can_reference = requires {
    typename with_reference<T>;
};

template <typename T>
concept dereferenceable = requires(T& t) {
    { *t } -> can_reference; // not required to be equality-preserving
};

template <typename T>
concept has_iterator_concept = requires {
    typename T::iterator_concept;
};

template <typename T>
concept has_iterator_category = requires {
    typename T::iterator_category;
};
// clang-format on

} // namespace detail

struct [[nodiscard]] input_iterator_tag {};

struct [[nodiscard]] output_iterator_tag {};

struct [[nodiscard]] forward_iterator_tag : input_iterator_tag {};

struct [[nodiscard]] bidirectional_iterator_tag : forward_iterator_tag {};

struct [[nodiscard]] random_access_iterator_tag : bidirectional_iterator_tag {};

struct [[nodiscard]] contiguous_iterator_tag : random_access_iterator_tag {};

// clang-format off
template <typename Iterator>
struct [[nodiscard]] iterator_traits {
    using iterator_concept  = Iterator::iterator_concept;
    using iterator_category = Iterator::iterator_category;
    using value_type        = Iterator::value_type;
    using difference_type   = Iterator::difference_type;
    using pointer           = Iterator::pointer;
    using reference         = Iterator::reference;
};

template <typename T>
struct [[nodiscard]] iterator_traits<T*> {
    using iterator_concept  = contiguous_iterator_tag;
    using iterator_category = random_access_iterator_tag;
    using value_type        = remove_cv_t<T>;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;
};

template <typename T>
using iter_difference_t = iterator_traits<remove_cvref_t<T>>::difference_type;

template <typename T>
concept weakly_incrementable = movable<T>
    and requires(T i) {
        typename iter_difference_t<T>;
        { ++i } -> same_as<T&>;
        { i++ };
    }
    and !same_as<T, bool>;
// clang-format on

namespace detail {

namespace iter_move_impl {

void iter_move() noexcept;

template <typename T>
concept class_or_enum = std::is_class_v<T> or std::is_enum_v<T>;

template <typename T>
concept unqualified_iter_move =
        class_or_enum<remove_cvref_t<T>> and requires(T&& t) { iter_move(forward<T>(t)); };

template <typename T>
concept move_deref = !unqualified_iter_move<T> and requires(T&& t) {
    { *t };
    requires is_lvalue_reference_v<decltype(*t)>;
};

template <typename T>
concept just_deref = !unqualified_iter_move<T> and !move_deref<T> and requires(T&& t) {
    { *t };
    requires(!is_lvalue_reference_v<decltype(*t)>);
};

struct fn {
    template <unqualified_iter_move Iterator>
    [[nodiscard]] constexpr decltype(auto) operator()(Iterator&& i) const noexcept {
        return iter_move(forward<Iterator>(i));
    }

    template <move_deref Iterator>
    [[nodiscard]] constexpr auto operator()(Iterator&& i) const noexcept
            -> decltype(move(*forward<Iterator>(i))) {
        return move(*forward<Iterator>(i));
    }

    template <just_deref Iterator>
    [[nodiscard]] constexpr auto operator()(Iterator&& i) const noexcept
            -> decltype(*forward<Iterator>(i)) {
        return *forward<Iterator>(i);
    }
};

} // namespace iter_move_impl

inline namespace cpo {
inline constexpr auto iter_move = iter_move_impl::fn{};
} // namespace cpo

} // namespace detail

template <detail::dereferenceable T>
    requires requires(T& t) {
        { detail::iter_move(t) } -> detail::can_reference;
    }
using iter_rvalue_reference_t = decltype(detail::iter_move(declval<T&>()));

template <typename Iterator>
concept input_or_output_iterator = requires(Iterator i) {
    { *i } -> detail::can_reference;
} and weakly_incrementable<Iterator>;

template <typename Iterator>
using iter_value_t = iterator_traits<remove_cvref_t<Iterator>>::value_type;
template <detail::dereferenceable T>
using iter_reference_t = decltype(*declval<T&>());
template <typename Iterator>
using iter_diff_t = iterator_traits<remove_cvref_t<Iterator>>::difference_type;

// clang-format off
template <typename Iterator>
concept indirectly_readable_impl = requires(const Iterator i) {
    typename iter_value_t<Iterator>;
    typename iter_reference_t<Iterator>;
    typename iter_rvalue_reference_t<Iterator>;
    { *i                   } -> same_as<iter_reference_t<Iterator>>;
    { detail::iter_move(i) } -> same_as<iter_rvalue_reference_t<Iterator>>;
} and common_reference_with<iter_reference_t<Iterator>&&, iter_value_t<Iterator>&>
  and common_reference_with<iter_reference_t<Iterator>&&, iter_rvalue_reference_t<Iterator>&&>
  and common_reference_with<iter_rvalue_reference_t<Iterator>&&, const iter_value_t<Iterator>&>;

template <typename Iterator>
concept indirectly_readable = indirectly_readable_impl<remove_cvref_t<Iterator>>;

template <indirectly_readable Iterator>
using iter_common_reference_t = std::common_reference_t<iter_reference_t<Iterator>, iter_value_t<Iterator>&>;

template <typename Iterator, typename T>
concept indirectly_writable = requires(Iterator&& i, T&& t) {
    *i                                                                          = static_cast<T&&>(t);
    *static_cast<Iterator&&>(i)                                                 = static_cast<T&&>(t);
    const_cast<const iter_reference_t<Iterator>&&>(*i)                          = static_cast<T&&>(t);
    const_cast<const iter_reference_t<Iterator>&&>(*static_cast<Iterator&&>(i)) = static_cast<T&&>(t);
};

template <typename T>
concept incrementable = std::regular<T>
    and weakly_incrementable<T>
    and requires(T t) {
        { t++ } -> same_as<T>;
    };
// clang-format on

namespace detail {

// clang-format off
template <bool>
struct iter_category {
    template <typename Iterator, typename Traits>
    using apply = typename Traits::iterator_category;
};
template <>
struct iter_category<false> {
    template <typename Iterator, typename Traits>
    using apply = random_access_iterator_tag;
};

template <bool>
struct iter_concept {
    template <typename Iterator, typename Traits>
    using apply = typename Traits::iterator_concept;
};
template <>
struct iter_concept<false> {
    template <typename Iterator, typename Traits>
    using apply = typename iter_category<has_iterator_category<Traits>>::template apply<Iterator, Traits>;
};
// clang-format on

} // namespace detail

template <typename Iterator, typename Traits = iterator_traits<Iterator>>
using iter_concept = typename detail::iter_concept<
        detail::has_iterator_concept<Traits>>::template apply<Iterator, Traits>;

// clang-format off
template <typename Iterator>
concept input_iterator = input_or_output_iterator<Iterator>
    and indirectly_readable<Iterator>
    and requires { typename iter_concept<Iterator>; }
    and derived_from<iter_concept<Iterator>, input_iterator_tag>;

template <typename Iterator, typename T>
concept output_iterator = input_or_output_iterator<Iterator>
    and indirectly_writable<Iterator, T>
    and requires(Iterator i, T&& t) {
        *i++ = static_cast<T&&>(t);
    };

template <typename Iterator>
concept forward_iterator = input_iterator<Iterator>
    and derived_from<iter_concept<Iterator>, forward_iterator_tag>
    and incrementable<Iterator>;

template <typename Iterator>
concept bidirectional_iterator = forward_iterator<Iterator>
    and derived_from<iter_concept<Iterator>, bidirectional_iterator_tag>
    and requires(Iterator i) {
        { --i } -> same_as<Iterator&>;
        { i-- } -> same_as<Iterator>;
    };

template <typename Iterator>
concept random_access_iterator = bidirectional_iterator<Iterator>
    and derived_from<iter_concept<Iterator>, random_access_iterator_tag>
    and std::totally_ordered<Iterator>
    and requires(Iterator i, const Iterator j, const iter_difference_t<Iterator> n) {
        { i += n } -> same_as<Iterator&>;
        { j +  n } -> same_as<Iterator>;
        { n +  j } -> same_as<Iterator>;
        { i -= n } -> same_as<Iterator&>;
        { j -  n } -> same_as<Iterator>;
        { j[n]   } -> same_as<iter_reference_t<Iterator>>;
    };

template <typename Iterator>
concept contiguous_iterator = random_access_iterator<Iterator>
    and derived_from<iter_concept<Iterator>, contiguous_iterator_tag>
    and is_lvalue_reference_v<iter_reference_t<Iterator>>
    and same_as<iter_value_t<Iterator>, remove_cvref_t<iter_reference_t<Iterator>>>;

template <typename T, typename U>
concept iterator_category_convertible_to =
        detail::has_iterator_category<iter_value_t<T>> and
        convertible_to<typename iterator_traits<T>::iterator_category, U>;

template <typename T>
concept has_random_access_iterator_category =
        iterator_category_convertible_to<T, random_access_iterator_tag>;

template <typename T>
concept addressable = pointer<T> or requires(T const x) { x.operator->(); };
// clang-format on

} // namespace salt::meta