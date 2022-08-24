#pragma once
#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>

#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/memory_stack.hpp>

namespace salt::detail {

// clang-format off
template <typename FreeList, typename AccessPolicy>
struct [[nodiscard]] Free_list_array final {
    // Creates sufficient elements to support up to given maximum node size. The actual amount is
    // calculated according to the policy.
    constexpr Free_list_array(Fixed_memory_stack& stack, std::byte const* end,
                              std::size_t max_node_size) noexcept
            : size_{AccessPolicy::index_from_size(max_node_size) - min_size + 1},
              array_{static_cast<FreeList*>(
                      stack.allocate(end, size_ * sizeof(FreeList), alignof(FreeList)))} {
        SALT_ASSERT(array_);
        for (std::size_t i = 0u; i < size_; ++i) {
            auto node_size = AccessPolicy::size_from_index(i + min_size);
            ::new (static_cast<void*>(array_ + i)) FreeList(node_size);
        }
    }

    constexpr Free_list_array(Free_list_array&& other) noexcept
            : array_{std::exchange(other.array_, nullptr)},
              size_ {std::exchange(other.size_ , 0u     )} {}

    ~Free_list_array() noexcept = default;

    constexpr Free_list_array& operator=(Free_list_array&& other) noexcept {
        array_ = std::exchange(other.array_, nullptr);
        size_  = std::exchange(other.size_, 0u);
        return *this;
    }

    constexpr FreeList& operator[](std::size_t node_size) const noexcept {
        auto index = AccessPolicy::index_from_size(node_size);
        if (index < min_size)
            index = min_size;
        return array_[index - min_size];
    }

    constexpr std::size_t size() const noexcept {
        return size_;
    }

    constexpr std::size_t max_node_size() const noexcept {
        return AccessPolicy::size_from_index(size_ + min_size - 1);
    }

private:
    static constexpr std::size_t min_size = AccessPolicy::index_from_size(FreeList::min_element_size);

    std::size_t size_;
    FreeList*   array_;
};
// clang-format on

// AccessPolicy that maps size to indices 1:1
struct [[nodiscard]] Identity_access_policy final {
    static constexpr std::size_t index_from_size(std::size_t size) noexcept {
        return size;
    }

    static constexpr std::size_t size_from_index(std::size_t index) noexcept {
        return index;
    }
};

// AccessPolicy that maps sizes to the integral log2.
// This creates more nodes and never wastes more than half the size.
struct [[nodiscard]] Log2_access_policy final {
    static constexpr std::size_t index_from_size(std::size_t size) noexcept {
        return ilog2_ceil(size);
    }

    static constexpr std::size_t size_from_index(std::size_t index) noexcept {
        return std::size_t{1} << index;
    }
};

} // namespace salt::detail
