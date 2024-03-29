#pragma once
#include <salt/foundation/slot_map_base.hpp>

#include <algorithm>
#include <vector>

namespace salt {

/// @see https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0661r0.pdf
// clang-format off
template <
    typename T,
    std::unsigned_integral KeyType                 = unsigned,
    template <typename...> typename ValueContainer = std::vector,
    template <typename...> typename KeyContainer   = ValueContainer
> requires slottable<T, KeyType, ValueContainer, KeyContainer>
// clang-format on
class [[nodiscard]] Slot_map : public Slot_map_base<T, KeyType, ValueContainer, KeyContainer> {
    using base             = Slot_map_base<T, KeyType, ValueContainer, KeyContainer>;
    using const_key_view   = decltype(std::declval<base const*>()->keys());
    using const_value_view = decltype(std::declval<base const*>()->values());
    using value_view       = decltype(std::declval<base*>()->values());

    using base::indices_;
    using base::keys_;
    using base::values_;
    using typename base::index_type;

    static_assert(std::ranges::borrowed_range<const_key_view>);
    static_assert(std::ranges::borrowed_range<const_value_view>);
    static_assert(std::ranges::borrowed_range<value_view>);
    static_assert(std::ranges::random_access_range<const_key_view>);
    static_assert(std::ranges::random_access_range<const_value_view>);
    static_assert(std::ranges::random_access_range<value_view>);

    using const_key_iterator   = std::ranges::iterator_t<const_key_view>;
    using const_value_iterator = std::ranges::iterator_t<const_value_view>;
    using value_iterator       = std::ranges::iterator_t<value_view>;

    static constexpr index_type free_idx_null = std::numeric_limits<KeyType>::max();
    index_type                  free_idx_     = free_idx_null;

public:
    using typename base::key_type;
    using typename base::size_type;

    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = T const*;
    using reference       = T&;
    using const_reference = T const&;
    using const_iterator  = Zip_iterator<const_key_iterator, const_value_iterator>;
    using iterator        = Zip_iterator<const_key_iterator, value_iterator>;
    using difference_type = std::iter_difference_t<iterator>;
    using emplace_result  = Emplace_result<key_type, value_type>;

    // clang-format off
    static constexpr size_type max_size() noexcept { return free_idx_null - index_type{1}; }

    constexpr iterator begin() noexcept;
    constexpr iterator end() noexcept;

    constexpr const_iterator begin() const noexcept;
    constexpr const_iterator end() const noexcept;

    constexpr const_iterator cbegin() const noexcept;
    constexpr const_iterator cend() const noexcept;

    constexpr size_type size() const noexcept;

    constexpr bool empty() const noexcept;
    constexpr void clear() noexcept;
    // clang-format on

    [[nodiscard]] constexpr key_type insert(value_type const& value)
        requires std::copy_constructible<value_type>;
    [[nodiscard]] constexpr key_type insert(value_type&& value)
        requires std::move_constructible<value_type>;

    // clang-format off
    template <typename... Args> requires std::constructible_from<value_type, Args&&...>
    [[nodiscard]] constexpr emplace_result emplace(Args&&... args);
    // clang-format on

    constexpr iterator erase(iterator it) noexcept;
    constexpr void     erase(key_type key) noexcept;

    constexpr value_type pop(key_type key) noexcept;
    constexpr void       swap(Slot_map& other) noexcept;

    constexpr iterator       access(key_type key) noexcept;
    constexpr const_iterator access(key_type key) const noexcept;

    constexpr iterator       find(key_type key) noexcept;
    constexpr const_iterator find(key_type key) const noexcept;

    // clang-format off
    constexpr pointer       data() noexcept       requires detail::has_data<Slot_map>;
    constexpr const_pointer data() const noexcept requires detail::has_data<Slot_map>;

    constexpr size_type capacity() const noexcept requires detail::has_capacity<Slot_map>;

    constexpr void shrink_to_fit() requires detail::has_shrink_to_fit<Slot_map>;

    constexpr void reserve(size_type size) requires detail::has_reserve<Slot_map>;
    // clang-format on

    constexpr T&       operator[](key_type key) noexcept;
    constexpr T const& operator[](key_type key) const noexcept;

    constexpr bool contains(key_type key) const noexcept;

    using base::keys;
    using base::values;

    constexpr bool operator==(Slot_map const& other) const noexcept;

private:
    constexpr index_type index(key_type key) const noexcept {
        SALT_ASSERT(key.idx < indices_.size());
        auto const idx = indices_[key.idx];
        SALT_ASSERT(idx < values_.size());
        return idx;
    }

    constexpr void erase_impl(index_type value_idx) noexcept;
    constexpr void erase_index_and_key(index_type value_idx) noexcept;
};

#include <salt/foundation/slot_map.inl>

#undef REQUIRES_HAS_RESERVE
#undef REQUIRES_HAS_CAPACITY
#undef REQUIRES_HAS_SHRINK_TO_FIT

} // namespace salt