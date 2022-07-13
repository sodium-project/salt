#pragma once

#include <salt/foundation/slot_map_base.hpp>

namespace salt {

#if SALT_CLANG_FULL_VER < 140000
// Clang 14.0 gives an error: 'out-of-line definition' when defining member function enabled with
// concept outside class. That seems a bug since the code should be correct.
// Related issue from LLVM repo: https://github.com/llvm/llvm-project/issues/56442
#    define CLANG_ERROR_OUT_OF_LINE_DEFINITION (1)
#endif

#ifdef CLANG_ERROR_OUT_OF_LINE_DEFINITION
#    define REQUIRES_HAS_RESERVE(SLOT_MAP)                                                         \
        requires detail::has_reserve<SLOT_MAP> {                                                   \
            reserve_impl(size);                                                                    \
        }
#    define REQUIRES_HAS_CAPACITY(SLOT_MAP)                                                        \
        requires detail::has_capacity<SLOT_MAP> {                                                  \
            return capacity_impl();                                                                \
        }
#    define REQUIRES_HAS_SHRINK_TO_FIT(SLOT_MAP)                                                   \
        requires detail::has_shrink_to_fit<SLOT_MAP> {                                             \
            shrink_to_fit_impl();                                                                  \
        }
#else
#    define REQUIRES_HAS_RESERVE(SLOT_MAP)       requires detail::has_reserve<SLOT_MAP>
#    define REQUIRES_HAS_CAPACITY(SLOT_MAP)      requires detail::has_capacity<SLOT_MAP>
#    define REQUIRES_HAS_SHRINK_TO_FIT(SLOT_MAP) requires detail::has_shrink_to_fit<SLOT_MAP>
#endif

/// @see https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0661r0.pdf
// clang-format off
template<
    typename T,
    std::unsigned_integral KeyType,
    template <typename...> typename ValueContainer = std::vector,
    template <typename...> typename KeyContainer   = ValueContainer
> requires slot_map_requires<T, KeyType, ValueContainer, KeyContainer>
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

    static_assert(cxx20::borrowed_range<const_key_view>);
    static_assert(cxx20::borrowed_range<const_value_view>);
    static_assert(cxx20::borrowed_range<value_view>);
    static_assert(cxx20::random_access_range<const_key_view>);
    static_assert(cxx20::random_access_range<const_value_view>);
    static_assert(cxx20::random_access_range<value_view>);

    using const_key_iterator   = std::ranges::iterator_t<const_key_view>;
    using const_value_iterator = std::ranges::iterator_t<const_value_view>;
    using value_iterator       = std::ranges::iterator_t<value_view>;

    static constexpr index_type free_idx_null = std::numeric_limits<KeyType>::max();
    index_type                  free_idx_     = free_idx_null;

public:
    using typename base::key_type;
    using typename base::size_type;

    using value_type      = T;
    using reference       = T&;
    using const_reference = T const&;

    using const_iterator = Zip_iterator<const_key_iterator, const_value_iterator>;
    using iterator       = Zip_iterator<const_key_iterator, value_iterator>;

    using difference_type = std::iter_difference_t<iterator>;
    using emplace_result  = Emplace_result<key_type, value_type>;

    // clang-format off
    constexpr iterator begin() noexcept { return {keys().begin(), values().begin()}; }
    constexpr iterator end  () noexcept { return {keys().end  (), values().end  ()}; }

    constexpr const_iterator begin() const noexcept { return {keys().begin(), values().begin()}; }
    constexpr const_iterator end  () const noexcept { return {keys().end  (), values().end  ()}; }

    constexpr const_iterator cbegin() const noexcept { return begin(); }
    constexpr const_iterator cend  () const noexcept { return end  (); }

    constexpr bool      empty() const noexcept { return begin() == end(); }
    constexpr size_type size () const noexcept {
        return static_cast<size_type>(cxx20::distance(begin(), end()));
    }

    static constexpr size_type max_size() noexcept { return free_idx_null - index_type{1}; }
    // clang-format on

    constexpr void      reserve(size_type size) REQUIRES_HAS_RESERVE(Slot_map);
    constexpr void      shrink_to_fit() REQUIRES_HAS_SHRINK_TO_FIT(Slot_map);
    constexpr size_type capacity() const noexcept REQUIRES_HAS_CAPACITY(Slot_map);

    constexpr void clear() noexcept;

    [[nodiscard]] constexpr key_type
    insert(value_type const& value) requires std::copy_constructible<value_type> {
        return emplace(value).key;
    }
    [[nodiscard]] constexpr key_type
    insert(value_type&& value) requires std::move_constructible<value_type> {
        return emplace(std::move(value)).key;
    }

    // clang-format off
    template <typename... Args> requires std::constructible_from<value_type, Args&&...>
    [[nodiscard]] constexpr emplace_result emplace(Args&&... args);
    // clang-format on

    constexpr iterator erase(iterator it) noexcept;
    constexpr void     erase(key_type key) noexcept {
            erase_impl(index(key));
    }

    constexpr value_type pop(key_type key) noexcept;
    constexpr void       swap(Slot_map& other) noexcept;

    constexpr const_iterator access(key_type key) const noexcept {
        return std::ranges::next(begin(), index(key));
    }
    constexpr iterator access(key_type key) noexcept {
        return std::ranges::next(begin(), index(key));
    };

    constexpr const_iterator find(key_type key) const noexcept;
    constexpr iterator       find(key_type key) noexcept;

    constexpr T& operator[](key_type key) noexcept {
        return values_[index(key)];
    }
    constexpr T const& operator[](key_type key) const noexcept {
        return values_[index(key)];
    }

    constexpr value_type* data() noexcept requires detail::has_data<Slot_map> {
        return values_.data();
    }
    constexpr value_type const* data() const noexcept requires detail::has_data<Slot_map> {
        return values_.data();
    }

    constexpr bool contains(key_type key) const noexcept {
        return find(key) != end();
    };

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

#ifdef CLANG_ERROR_OUT_OF_LINE_DEFINITION
    constexpr void      reserve_impl(size_type size);
    constexpr void      shrink_to_fit_impl();
    constexpr size_type capacity_impl() const noexcept;
#endif
};

#include <salt/foundation/slot_map.inl>

#undef REQUIRES_HAS_RESERVE
#undef REQUIRES_HAS_CAPACITY
#undef REQUIRES_HAS_SHRINK_TO_FIT

} // namespace salt