#pragma once
#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>

#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/memory_stack.hpp>

namespace salt::detail {

// clang-format off
template <typename AccessPolicy>
concept access_policy =
    requires {
        { AccessPolicy::index_from_size(1) } -> std::same_as<std::size_t>;
        { AccessPolicy::size_from_index(1) } -> std::same_as<std::size_t>;
    };
// clang-format on

// An array of Memory_list types indexed via size, AccessPolicy does necessary conversions requires
// trivial destructible MemoryList type.
template <typename MemoryList, access_policy AccessPolicy>
struct [[nodiscard]] Memory_list_array final {
    using access_policy_type = AccessPolicy;
    using memory_list        = MemoryList;
    using size_type          = typename memory_list::size_type;
    using const_iterator     = typename memory_list::const_iterator;

    constexpr Memory_list_array(Memory_stack& stack, const_iterator begin, size_type max_node_size) noexcept
            : size_ {access_policy_type::index_from_size(max_node_size) - min_size + 1},
              array_{static_cast<memory_list*>(
                      stack.allocate(begin, size_ * sizeof(memory_list), alignof(memory_list)))} {
        SALT_ASSERT(array_);
        for (size_type i = 0u; i < size_; ++i) {
            auto node_size = access_policy_type::size_from_index(i + min_size);
            ::new (static_cast<void*>(array_ + i)) memory_list{node_size};
        }
    }

    constexpr ~Memory_list_array() = default;

    // clang-format off
    constexpr Memory_list_array(Memory_list_array&& other) noexcept
            : array_{std::exchange(other.array_, nullptr)},
              size_ {std::exchange(other.size_ , 0u     )} {}

    constexpr Memory_list_array& operator=(Memory_list_array&& other) noexcept {
        array_ = std::exchange(other.array_, nullptr);
        size_  = std::exchange(other.size_ , 0u);
        return *this;
    }
    // clang-format on

    constexpr memory_list& operator[](std::size_t node_size) const noexcept {
        auto index = access_policy_type::index_from_size(node_size);
        if (index < min_size)
            index = min_size;
        return array_[index - min_size];
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

    constexpr size_type max_node_size() const noexcept {
        return access_policy_type::size_from_index(size_ + min_size - 1);
    }

private:
    static constexpr size_type min_size =
            access_policy_type::index_from_size(memory_list::min_size);

    size_type    size_;
    memory_list* array_;
};

// AccessPolicy that maps size to indices 1:1.
struct [[nodiscard]] Identity_access_policy final {
    static constexpr std::size_t index_from_size(std::size_t size) noexcept {
        return size;
    }

    static constexpr std::size_t size_from_index(std::size_t index) noexcept {
        return index;
    }
};

// AccessPolicy that maps sizes to the integral log2. This creates more nodes and never wastes more
// than half the size.
struct [[nodiscard]] Log2_access_policy final {
    static constexpr std::size_t index_from_size(std::size_t size) noexcept {
        return ilog2_ceil(size);
    }

    static constexpr std::size_t size_from_index(std::size_t index) noexcept {
        return std::size_t{1} << index;
    }
};

} // namespace salt::detail
