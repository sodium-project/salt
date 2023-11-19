#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/memory/align.hpp>

namespace salt::memory {

namespace detail {

// clang-format off
template <typename Allocator, typename T = meta::template_parameter_t<Allocator>>
concept has_cxx98_construct =
    requires(Allocator allocator, T* ptr, T const& value) {
        { allocator.construct(ptr, value) } -> meta::same_as<void>;
    };
template <typename Allocator, typename T = int>
concept has_cxx11_construct =
    requires(Allocator allocator, T* ptr) {
        { allocator.construct(ptr) } -> meta::same_as<void>;
    };

template <typename Allocator, typename T = meta::template_parameter_t<Allocator>>
concept has_cxx98_destroy =
    requires(Allocator allocator, T* ptr) {
        { allocator.destroy(ptr) } -> meta::same_as<void>;
    };
template <typename Allocator, typename T = int>
concept has_cxx11_destroy =
    requires(Allocator allocator, T* ptr) {
        { allocator.destroy(ptr) } -> meta::same_as<void>;
    };

template <typename Allocator>
concept has_construct =
    has_cxx98_construct<Allocator> or
    has_cxx11_construct<Allocator>;
template <typename Allocator>
concept has_destroy =
    has_cxx98_destroy<Allocator> or
    has_cxx11_destroy<Allocator>;

template <typename Allocator>
concept deprecated_allocator =
    has_construct<Allocator> and
    has_destroy  <Allocator>;
// clang-format on

} // namespace detail

// Traits variable that checks whether an `allocator` can be used as a `concept raw_allocator`.
// It checks for deprecated `construct` and `destroy` functions, if provided, it cannot be used
// since it would not be called.
// Specialize it for custom `allocator` types to override this check. For example:
// ```c++
// template <typename T>
// inline constexpr bool allocator_is_raw_allocator<std::allocator<T>> = true;
// ```
template <typename Allocator>
inline constexpr bool allocator_is_raw_allocator = not detail::deprecated_allocator<Allocator>;

template <typename Allocator>
concept raw_allocator = allocator_is_raw_allocator<Allocator>;
template <typename Allocator>
inline constexpr bool is_raw_allocator = raw_allocator<Allocator>;

namespace detail {

// clang-format off
template <typename Allocator>
concept has_allocate =
    requires(Allocator&& allocator, std::size_t size) {
        { allocator.allocate(size) };
    };

template <typename Allocator>
concept has_void_deallocate =
    requires(Allocator&& allocator, void* ptr, std::size_t size) {
        { allocator.deallocate(ptr, size) };
    };

template <typename Allocator, typename T = meta::template_parameter_t<Allocator>>
concept has_typed_deallocate =
    requires(Allocator&& allocator, T* ptr, std::size_t size) {
        { allocator.deallocate(ptr, size) };
    };

template <typename Allocator>
concept has_allocate_node =
    requires(Allocator&& allocator, std::size_t size, std::size_t align) {
        { allocator.allocate_node(size, align) };
    };

template <typename Allocator>
concept has_deallocate_node =
    requires(Allocator&& allocator, void* ptr, std::size_t size, std::size_t align) {
        { allocator.deallocate_node(ptr, size, align) };
    };

template <typename Allocator>
concept has_allocate_array =
    requires(Allocator&& allocator, std::size_t count, std::size_t size, std::size_t align) {
        { allocator.allocate_array(count, size, align) };
    };

template <typename Allocator>
concept has_deallocate_array =
    requires(Allocator&& allocator, void* ptr, std::size_t count, std::size_t size, std::size_t align) {
        { allocator.deallocate_array(ptr, count, size, align) };
    };

template <typename Allocator>
concept has_max_node_size =
    requires(Allocator&& allocator) {
        { allocator.max_node_size() } noexcept -> meta::integer;
    };

template <typename Allocator>
concept has_max_array_size =
    requires(Allocator&& allocator) {
        { allocator.max_array_size() } noexcept -> meta::integer;
    };

template <typename Allocator>
concept has_max_alignment =
    requires(Allocator&& allocator) {
        { allocator.max_alignment() } noexcept -> meta::integer;
    };
// clang-format on

} // namespace detail

// clang-format off
template <typename Allocator>
concept empty_allocator =
    meta::is_empty_v           <Allocator> and
    meta::default_constructible<Allocator>;
template <typename Allocator>
inline constexpr bool is_empty_allocator = empty_allocator<Allocator>;

template <typename Allocator>
concept stateful_allocator =
    not empty_allocator<Allocator>                and
    requires { typename Allocator::is_stateful; } and
    Allocator::is_stateful::value == true;
template <typename Allocator>
inline constexpr bool is_stateful_allocator = stateful_allocator<Allocator>;

template <typename Allocator>
struct [[nodiscard]] allocator_traits final {
    using allocator_type  = Allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     = meta::condition<is_stateful_allocator<Allocator>,
                                            meta::true_type, meta::false_type>;

    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment) noexcept
    {
        if constexpr (detail::has_allocate_node<allocator_type>)
            return allocator.allocate_node(size, alignment);
        else if constexpr (detail::has_allocate<allocator_type>)
            return static_cast<void*>(allocator.allocate(size));
        else
            return nullptr;
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment) noexcept
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
        else if constexpr (detail::has_void_deallocate<allocator_type>)
            allocator.deallocate(node, size);            
        else if constexpr (detail::has_typed_deallocate<allocator_type>)
            allocator.deallocate(static_cast<allocator_type::value_type*>(node), size);
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

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        if constexpr (detail::has_max_node_size<allocator_type>)
            return allocator.max_node_size();
        else
            return size_type(-1) / sizeof(std::byte);
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        if constexpr (detail::has_max_array_size<allocator_type>)
            return allocator.max_array_size();
        else
            return max_node_size(allocator);
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        if constexpr (detail::has_max_alignment<allocator_type>)
            return allocator.max_alignment();
        else
            return memory::max_alignment;
    }
};
// clang-format on

namespace detail {

// clang-format off
template <typename Allocator>
concept has_try_allocate_node =
    requires(Allocator&& allocator, std::size_t size, std::size_t align) {
        { allocator.try_allocate_node(size, align) } noexcept;
    };

template <typename Allocator>
concept has_try_deallocate_node =
    requires(Allocator&& allocator, void* ptr, std::size_t size, std::size_t align) {
        { allocator.try_deallocate_node(ptr, size, align) } noexcept;
    };

template <typename Allocator>
concept has_try_allocate_array =
    requires(Allocator&& allocator, std::size_t count, std::size_t size, std::size_t align) {
        { allocator.try_allocate_array(count, size, align) } noexcept;
    };

template <typename Allocator>
concept has_try_deallocate_array =
    requires(Allocator&& allocator, void* ptr, std::size_t count, std::size_t size, std::size_t align) {
        { allocator.try_deallocate_array(ptr, count, size, align) } noexcept;
    };
// clang-format on

} // namespace detail

// clang-format off
template <typename Allocator>
struct [[nodiscard]] composable_traits final {
    using allocator_type  = typename allocator_traits<Allocator>::allocator_type;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    static constexpr void*
    try_allocate_node(allocator_type& allocator,
                      size_type       size     ,
                      size_type       alignment) noexcept
        requires detail::has_try_allocate_node<allocator_type>
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
                        size_type       alignment) noexcept
        requires detail::has_try_deallocate_node<allocator_type>
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
};

template <typename Allocator>
concept composable_allocator =
    raw_allocator                  <Allocator> and
    detail::has_try_allocate_node  <Allocator> and
    detail::has_try_deallocate_node<Allocator>;
template <typename Allocator>
inline constexpr bool is_composable_allocator = composable_allocator<Allocator>;
// clang-format on

} // namespace salt::memory