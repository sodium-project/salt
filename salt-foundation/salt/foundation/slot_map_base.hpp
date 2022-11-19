#pragma once

#include <salt/foundation/zip_iterator.hpp>
#include <salt/meta.hpp>

#if SALT_TARGET(APPLE)
#    define SALT_LIBCPP_HAS_NO_RANGES (1)
#else
#    include <algorithm>
#endif

namespace salt::ranges {
#ifdef SALT_LIBCPP_HAS_NO_RANGES
using std::distance;
using std::is_permutation;
#else
using std::ranges::distance;
using std::ranges::is_permutation;
#endif
} // namespace salt::ranges

namespace salt {

template <typename Container, typename Friend> struct Container_view : private Container {
    friend Friend;
    using Container::begin;
    using Container::end;

    // clang-format off
    constexpr auto cbegin() const noexcept { return begin(); }
    constexpr auto cend  () const noexcept { return end  (); }

    constexpr bool empty () const noexcept { return begin() == end();                 }
    constexpr auto size  () const noexcept { return ranges::distance(begin(), end()); }
    // clang-format on

    constexpr decltype(auto) operator[](typename Container::size_type idx) noexcept {
        return begin()[idx];
    }
    constexpr decltype(auto) operator[](typename Container::size_type idx) const noexcept {
        return begin()[idx];
    }
};

template <std::unsigned_integral I> struct Key final {
    using index_type = I;

    index_type     idx;
    constexpr auto operator<=>(Key const&) const noexcept = default;
};

template <typename Key, typename Value> struct Emplace_result final {
    Key    key;
    Value& ref;
};

namespace detail {

// clang-format off
template <typename SlotMap>
concept has_reserve =
    has_reserve<typename SlotMap::value_container> and
    has_reserve<typename SlotMap::key_container>;

template <typename SlotMap>
concept has_capacity =
    has_capacity<typename SlotMap::value_container> and
    has_capacity<typename SlotMap::key_container>;

template <typename SlotMap>
concept has_shrink_to_fit =
    has_shrink_to_fit<typename SlotMap::value_container> and
    has_shrink_to_fit<typename SlotMap::key_container>;

template <typename SlotMap>
concept has_data = has_data<typename SlotMap::value_container>;
// clang-format on

} // namespace detail

// clang-format off
template <
    typename T,
    typename I,
    template <typename...> typename ValueContainer,
    template <typename...> typename KeyContainer
>
concept slottable =
    std::unsigned_integral<I>               and
    std::is_nothrow_move_constructible_v<T> and
    std::is_nothrow_move_assignable_v<T>    and

    requires(ValueContainer<T> c) { { std::ranges::swap(c, c) } noexcept; } and
    requires(KeyContainer  <I> c) { { std::ranges::swap(c, c) } noexcept; };
// clang-format on

// clang-format off
template <
    typename T,
    std::unsigned_integral I,
    template <typename...> typename ValueContainer,
    template <typename...> typename KeyContainer
> requires slottable<T, I, ValueContainer, KeyContainer>
// clang-format on
struct [[nodiscard]] Slot_map_base {
protected:
    using key_type   = Key<I>;
    using index_type = typename key_type::index_type;

    using value_container = ValueContainer<T>;
    using index_container = KeyContainer<index_type>;
    using key_container   = KeyContainer<key_type>;
    using value_view      = Container_view<value_container, Slot_map_base>;
    using index_view      = Container_view<index_container, Slot_map_base>;
    using key_view        = Container_view<key_container, Slot_map_base>;

    using size_type = std::common_type_t<typename value_container::size_type,
                                         typename index_container::size_type,
                                         typename key_container::size_type>;

    value_container values_;
    index_container indices_;
    key_container   keys_;

public:
    constexpr key_view const& keys() const noexcept {
        return static_cast<key_view const&>(keys_);
    }

    constexpr value_view& values() noexcept {
        return static_cast<value_view&>(values_);
    }
    constexpr value_view const& values() const noexcept {
        return static_cast<value_view const&>(values_);
    }
};

} // namespace salt