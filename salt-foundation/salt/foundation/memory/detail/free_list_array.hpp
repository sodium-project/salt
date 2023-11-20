#pragma once

#include <salt/foundation/memory/construct_at.hpp>
#include <salt/foundation/memory/detail/fixed_stack.hpp>

namespace salt::memory::detail {

// clang-format off
template <typename AccessPolicy>
concept access_policy =
    requires {
        { AccessPolicy::index_from_size(1) } -> meta::same_as<std::size_t>;
        { AccessPolicy::size_from_index(1) } -> meta::same_as<std::size_t>;
    };
// clang-format on

// An array of `free_list` types indexed via size, AccessPolicy does necessary conversions.
template <meta::trivially_destructible FreeList, access_policy AccessPolicy>
struct [[nodiscard]] free_list_array final {
    using access_policy  = AccessPolicy;
    using free_list_type = FreeList;
    using size_type      = typename free_list_type::size_type;
    using iterator       = typename free_list_type::iterator;
    using const_iterator = typename free_list_type::const_iterator;

    // clang-format off
    constexpr free_list_array(fixed_stack&   stack,
                              const_iterator begin,
                              size_type      max_node_size) noexcept
            : size_ {access_policy::index_from_size(max_node_size) - min_node_size + 1},
              array_{static_cast<free_list_type*>(stack.allocate(
                      begin, size_ * sizeof(free_list_type), alignof(free_list_type)))}
    {
        SALT_ASSERT(array_, "Insufficient memory for free lists.");
        for (size_type i = 0u; i < size_; ++i) {
            auto node_size = access_policy::size_from_index(i + min_node_size);
            construct_at(array_ + i, node_size);
        }
    }

    constexpr ~free_list_array() = default;

    constexpr free_list_array(free_list_array&& other) noexcept
            : size_ {utility::exchange(other.size_ , 0u     )},
              array_{utility::exchange(other.array_, nullptr)} {}

    constexpr free_list_array& operator=(free_list_array&& other) noexcept {
        size_  = utility::exchange(other.size_ , 0u);
        array_ = utility::exchange(other.array_, nullptr);
        return *this;
    }
    // clang-format on

    constexpr free_list_type& operator[](std::size_t node_size) const noexcept {
        auto index = access_policy::index_from_size(node_size);
        if (index < min_node_size)
            index = min_node_size;
        return array_[index - min_node_size];
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

    constexpr size_type max_node_size() const noexcept {
        return access_policy::size_from_index(size_ + min_node_size - 1);
    }

private:
    static constexpr size_type min_node_size =
            access_policy::index_from_size(free_list_type::min_node_size);

    size_type       size_;
    free_list_type* array_;
};

// AccessPolicy that maps size to indices 1:1.
struct [[nodiscard]] identity_access_policy final {
    static constexpr auto index_from_size(std::size_t size) noexcept {
        return size;
    }

    static constexpr auto size_from_index(std::size_t index) noexcept {
        return index;
    }
};

// AccessPolicy that maps sizes to the integral log2. This creates more nodes and never wastes more
// than half the size.
struct [[nodiscard]] log2_access_policy final {
    static constexpr auto index_from_size(std::size_t size) noexcept {
        return ilog2_ceil(size);
    }

    static constexpr auto size_from_index(std::size_t index) noexcept {
        return std::size_t{1} << index;
    }
};

} // namespace salt::memory::detail