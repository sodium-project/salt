#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/fast_terminate.hpp>
#include <salt/memory/detail/align.hpp>

namespace salt {

namespace detail {

// clang-format off
template <typename Allocator>
concept has_allocate_node =
    requires(Allocator allocator, std::size_t size, std::size_t alignment) {
        { allocator.allocate_node(size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_deallocate_node =
    requires(Allocator allocator, void* node, std::size_t size, std::size_t alignment) {
        { allocator.deallocate_node(node, size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_allocate_array =
    requires(Allocator allocator, std::size_t count, std::size_t size, std::size_t alignment) {
        { allocator.allocate_array(count, size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_deallocate_array =
    requires(Allocator allocator, void* array, std::size_t count, std::size_t size,
             std::size_t alignment) {
        { allocator.deallocate_array(array, count, size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_max_node_size =
    requires(Allocator const allocator) {
        { allocator.max_node_size() } noexcept -> std::same_as<std::size_t>;
    };

template <typename Allocator>
concept has_max_array_size =
    requires(Allocator const allocator) {
        { allocator.max_array_size() } noexcept -> std::same_as<std::size_t>;
    };

template <typename Allocator>
concept has_max_alignment =
    requires(Allocator const allocator) {
        { allocator.max_alignment() } noexcept -> std::same_as<std::size_t>;
    };
// clang-format on

} // namespace detail

// clang-format off
template <typename Allocator> struct [[nodiscard]] allocator_traits final {
    using allocator_type  = Allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     = typename allocator_type::is_stateful;

    static constexpr bool has_allocate_node   = detail::has_allocate_node<allocator_type>;
    static constexpr bool has_deallocate_node = detail::has_deallocate_node<allocator_type>;

    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment) requires has_allocate_node {
        return allocator.allocate_node(size, alignment);
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment) {
        if constexpr (detail::has_allocate_array<allocator_type>)
            return allocator.allocate_array(count, size, alignment);
        else if constexpr (detail::has_allocate_node<allocator_type>)
            return allocate_node(allocator, count * size, alignment);
    }

    static constexpr void
    deallocate_node(allocator_type& allocator,
                    void*           node     ,
                    size_type       size     ,
                    size_type       alignment) noexcept requires has_deallocate_node {
        allocator.deallocate_node(node, size, alignment);
    }

    static constexpr void
    deallocate_array(allocator_type& allocator,
                     void*           array    ,
                     size_type       count    ,
                     size_type       size     ,
                     size_type       alignment) noexcept {
        if constexpr (detail::has_deallocate_array<allocator_type>)
            allocator.deallocate_array(array, count, size, alignment);
        else if constexpr (detail::has_deallocate_node<allocator_type>)
            deallocate_node(allocator, array, count * size, alignment);
    }

    static constexpr size_type max_node_size(allocator_type const& allocator) {
        if constexpr (detail::has_max_node_size<allocator_type>)
            return allocator.max_node_size();
        else
            return size_type(-1) / sizeof(std::byte);
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) {
        if constexpr (detail::has_max_node_size<allocator_type>)
            return allocator.max_array_size();
        else
            return max_node_size(allocator);
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) {
        if constexpr (detail::has_max_node_size<allocator_type>)
            return allocator.max_alignment();
        else
            return detail::max_alignment;
    }
};
// clang-format on

} // namespace salt