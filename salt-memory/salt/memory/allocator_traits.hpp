#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/fast_terminate.hpp>
#include <salt/memory/detail/align.hpp>

namespace salt {

namespace detail {

// clang-format off
template <typename Allocator, typename T = int>
concept has_construct =
    requires(Allocator allocator) {
        { allocator.construct(std::declval<T*>(), std::declval<T>()) } -> std::same_as<void>;
    };

template <typename Allocator, typename T = int>
concept has_destroy =
    requires(Allocator allocator) {
        { allocator.destroy(std::declval<T*>()) } -> std::same_as<void>;
    };

template <typename Allocator>
concept standard_allocator =
    not has_construct<Allocator> and
    not has_destroy  <Allocator>;
// clang-format on

} // namespace detail

template <typename Allocator>
concept raw_allocator = detail::standard_allocator<Allocator>;
template <typename Allocator>
static constexpr inline bool is_raw_allocator = raw_allocator<Allocator>;

namespace detail {

// clang-format off
template <typename Allocator, typename T = typename Allocator::value_type>
concept has_allocate =
    requires(Allocator&& allocator, std::size_t size) {
        { allocator.allocate(size) } -> std::same_as<T*>;
    };

template <typename Allocator>
concept has_deallocate =
    requires(Allocator&& allocator, void* ptr, std::size_t size) {
        { allocator.deallocate(ptr, size) } -> std::same_as<void>;
    };

template <typename Allocator>
concept has_allocate_node =
    requires(Allocator&& allocator, std::size_t size, std::size_t alignment) {
        { allocator.allocate_node(size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_deallocate_node =
    requires(Allocator&& allocator, void* node, std::size_t size, std::size_t alignment) {
        { allocator.deallocate_node(node, size, alignment) } -> std::same_as<void>;
    };

template <typename Allocator>
concept has_allocate_array =
    requires(Allocator&& allocator, std::size_t count, std::size_t size, std::size_t alignment) {
        { allocator.allocate_array(count, size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_deallocate_array =
    requires(Allocator&& allocator, void* array, std::size_t count, std::size_t size, std::size_t alignment) {
        { allocator.deallocate_array(array, count, size, alignment) } -> std::same_as<void>;
    };

template <typename Allocator>
concept has_max_node_size =
    requires(Allocator&& allocator) {
        { allocator.max_node_size() } noexcept -> std::same_as<std::size_t>;
    };

template <typename Allocator>
concept has_max_array_size =
    requires(Allocator&& allocator) {
        { allocator.max_array_size() } noexcept -> std::same_as<std::size_t>;
    };

template <typename Allocator>
concept has_max_alignment =
    requires(Allocator&& allocator) {
        { allocator.max_alignment() } noexcept -> std::same_as<std::size_t>;
    };
// clang-format on

} // namespace detail

// clang-format off
template <typename Allocator>
concept empty_allocator =
    std::is_empty_v                <Allocator> and
    std::is_default_constructible_v<Allocator>;
template <typename Allocator>
static constexpr inline bool is_empty_allocator = empty_allocator<Allocator>;

template <typename Allocator>
concept stateful_allocator =
    not empty_allocator<Allocator>                and
    requires { typename Allocator::is_stateful; } and
    Allocator::is_stateful::value == true;
template <typename Allocator>
static constexpr inline bool is_stateful_allocator = stateful_allocator<Allocator>;
// clang-format on

template <raw_allocator Allocator> struct [[nodiscard]] allocator_traits final {
    using allocator_type  = Allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     =
            std::conditional_t<is_stateful_allocator<Allocator>, std::true_type, std::false_type>;

    // clang-format off
    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment)
    {
        if constexpr (detail::has_allocate_node<allocator_type>)
            return allocator.allocate_node(size, alignment);
        else if constexpr (detail::has_allocate<allocator_type>)
            return static_cast<void*>(allocator.allocate(size));
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment)
    {
        if constexpr (detail::has_allocate_array<allocator_type>)
            return allocator.allocate_array(count, size, alignment);
        else
            return allocate_node(allocator, count * size, alignment);
    }

    static constexpr void
    deallocate_node(allocator_type& allocator,
                    void*           node     ,
                    size_type       size     ,
                    size_type       alignment) noexcept
    {
        if constexpr (detail::has_deallocate_node<allocator_type>)
            allocator.deallocate_node(node, size, alignment);
        else if constexpr (detail::has_deallocate<allocator_type>)
            allocator.deallocate(node, size);
    }

    static constexpr void
    deallocate_array(allocator_type& allocator,
                     void*           array    ,
                     size_type       count    ,
                     size_type       size     ,
                     size_type       alignment) noexcept
    {
        if constexpr (detail::has_deallocate_array<allocator_type>)
            allocator.deallocate_array(array, count, size, alignment);
        else
            deallocate_node(allocator, array, count * size, alignment);
    }
    // clang-format on

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

namespace detail {

// clang-format off
template <typename Allocator>
concept has_try_allocate_node =
    requires(Allocator&& allocator, std::size_t size, std::size_t alignment) {
        { allocator.try_allocate_node(size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_try_deallocate_node =
    requires(Allocator&& allocator, void* node, std::size_t size, std::size_t alignment) {
        { allocator.try_deallocate_node(node, size, alignment) } -> std::same_as<bool>;
    };

template <typename Allocator>
concept has_try_allocate_array =
    requires(Allocator&& allocator, std::size_t count, std::size_t size, std::size_t alignment) {
        { allocator.try_allocate_array(count, size, alignment) } -> std::same_as<void*>;
    };

template <typename Allocator>
concept has_try_deallocate_array =
    requires(Allocator&& allocator, void* array, std::size_t count, std::size_t size, std::size_t alignment) {
        { allocator.try_deallocate_array(array, count, size, alignment) } -> std::same_as<bool>;
    };
// clang-format on

} // namespace detail

template <raw_allocator Allocator> struct [[nodiscard]] composable_traits final {
    using allocator_type  = typename allocator_traits<Allocator>::allocator_type;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    static constexpr void*
    try_allocate_node(allocator_type& allocator,
                      size_type       size     ,
                      size_type       alignment) noexcept requires
        detail::has_try_allocate_node<allocator_type>
    {
        return allocator.try_allocate_node(size, alignment);
    }

    static constexpr void*
    try_allocate_array(allocator_type& allocator,
                       size_type       count    ,
                       size_type       size     ,
                       size_type       alignment) noexcept
    {
        if constexpr (detail::has_try_allocate_array<allocator_type>)
            return allocator.try_allocate_array(count, size, alignment);
        else
            return try_allocate_node(allocator, count * size, alignment);
    }

    static constexpr bool
    try_deallocate_node(allocator_type& allocator,
                        void*           node     ,
                        size_type       size     ,
                        size_type       alignment) noexcept requires
        detail::has_try_deallocate_node<allocator_type>
    {
        return allocator.try_deallocate_node(node, size, alignment);
    }

    static constexpr bool
    try_deallocate_array(allocator_type& allocator,
                         void*           array    ,
                         size_type       count    ,
                         size_type       size     ,
                         size_type       alignment) noexcept
    {
        if constexpr (detail::has_try_deallocate_array<allocator_type>)
            return allocator.try_deallocate_array(array, count, size, alignment);
        else
            return try_deallocate_node(allocator, array, count * size, alignment);
    }
    // clang-format on
};

// clang-format off
template <typename Allocator>
concept composable_allocator =
    raw_allocator                  <Allocator> and
    detail::has_try_allocate_node  <Allocator> and
    detail::has_try_deallocate_node<Allocator>;
template <typename Allocator>
static constexpr inline bool is_composable_allocator = composable_allocator<Allocator>;
// clang-format on

} // namespace salt