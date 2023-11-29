#pragma once
#include <salt/foundation/memory/allocator_storage.hpp>
#include <salt/foundation/memory/allocator_traits.hpp>
#include <salt/foundation/memory/threading.hpp>

namespace salt::memory {

namespace detail {

// clang-format off
template <typename Allocator>
concept has_propagate_on_container_swap =
    requires {
       typename Allocator::propagate_on_container_swap;
    };

template <typename Allocator>
concept has_propagate_on_container_move_assignment =
    requires {
       typename Allocator::propagate_on_container_move_assignment;
    };

template <typename Allocator>
concept has_propagate_on_container_copy_assignment =
    requires {
       typename Allocator::propagate_on_container_copy_assignment;
    };

template <typename AllocatorReference>
concept any_reference = meta::same_as<AllocatorReference, any_allocator_reference>;
// clang-format on

} // namespace detail

// clang-format off
template <raw_allocator RawAllocator>
struct [[nodiscard]] propagation_traits final {
    using propagate_on_container_swap            =
            meta::condition<detail::has_propagate_on_container_swap<RawAllocator>,
                            meta::true_type, meta::false_type>;
    using propagate_on_container_move_assignment =
            meta::condition<detail::has_propagate_on_container_move_assignment<RawAllocator>,
                            meta::true_type, meta::false_type>;
    using propagate_on_container_copy_assignment =
            meta::condition<detail::has_propagate_on_container_copy_assignment<RawAllocator>,
                            meta::true_type, meta::false_type>;

    template <typename Allocator>
    static constexpr Allocator select_on_container_copy_construction(Allocator const& allocator) {
        return allocator;
    }
};
// clang-format on

template <typename Derived, typename Base>
concept not_derived_from = not meta::derived_from<Derived, Base>;

template <typename T, raw_allocator RawAllocator>
class [[nodiscard]] std_allocator_adapter : protected allocator_reference<RawAllocator> {
    using allocator_reference = allocator_reference<RawAllocator>;
    using allocator_traits    = allocator_traits   <RawAllocator>;
    using propagation_traits  = propagation_traits <RawAllocator>;

    static constexpr bool is_any_reference      = detail::any_reference<allocator_reference>;
    static constexpr bool is_shared_allocator   = memory::is_shared_allocator  <RawAllocator>::value;
    static constexpr bool is_stateful_allocator = memory::is_stateful_allocator<RawAllocator>::value;

public:
    using allocator_type  = typename allocator_reference::allocator_type;
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    // clang-format off
    using propagate_on_container_swap            =
            typename propagation_traits::propagate_on_container_swap;
    using propagate_on_container_move_assignment =
            typename propagation_traits::propagate_on_container_move_assignment;
    using propagate_on_container_copy_assignment =
            typename propagation_traits::propagate_on_container_move_assignment;

    constexpr std_allocator_adapter() noexcept requires(not is_stateful_allocator)
            : allocator_reference{allocator_type{}} {}

    constexpr explicit(false) std_allocator_adapter(allocator_reference const& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename Allocator>
        requires not_derived_from<Allocator, std_allocator_adapter>
    constexpr explicit(false) std_allocator_adapter(Allocator& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename Allocator>
        requires not_derived_from<Allocator, std_allocator_adapter>
    constexpr explicit(false) std_allocator_adapter(Allocator const& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename U>
    constexpr std_allocator_adapter(std_allocator_adapter<U, RawAllocator>& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename U>
    constexpr std_allocator_adapter(std_allocator_adapter<U, RawAllocator> const& allocator) noexcept
            : allocator_reference{allocator} {}
    // clang-format on

    // Implicit conversion from any other `allocator_storage` is forbidden to prevent accidentally
    // wrapping another `allocator_storage` inside the `allocator_reference`.
    template <typename Storage, typename Mutex>
    constexpr std_allocator_adapter(allocator_storage<Storage, Mutex>&) = delete;

    [[nodiscard]] constexpr T* allocate(size_type n) noexcept {
        if constexpr (is_any_reference)
            return static_cast<T*>(any_allocate_impl(n));
        else
            return static_cast<T*>(allocate_impl(n));
    }

    constexpr void deallocate(T* ptr, size_type n) noexcept {
        if constexpr (is_any_reference)
            return any_deallocate_impl(ptr, n);
        else
            return deallocate_impl(ptr, n);
    }

    constexpr auto select_on_container_copy_construction() const noexcept {
        return propagation_traits::select_on_container_copy_construction(*this);
    }

    constexpr decltype(auto) allocator() noexcept {
        return allocator_reference::allocator();
    }
    constexpr decltype(auto) allocator() const noexcept {
        return allocator_reference::allocator();
    }

    template <typename T1, typename T2, typename Allocator>
    friend constexpr bool operator==(std_allocator_adapter<T1, Allocator> const&,
                                     std_allocator_adapter<T2, Allocator> const&) noexcept;

private:
    // clang-format off
#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
    [[clang::always_inline]]
#endif
    constexpr void* allocate_impl(size_type n) noexcept {
        if (1u == n)
            return allocator_reference::allocate_node(sizeof(T), alignof(T));

        return allocator_reference::allocate_array(n, sizeof(T), alignof(T));
    }

#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
    [[clang::always_inline]]
#endif
    constexpr void deallocate_impl(void* ptr, size_type n) noexcept {
        if (1u == n)
            return allocator_reference::deallocate_node(ptr, sizeof(T), alignof(T));

        return allocator_reference::deallocate_array(ptr, n, sizeof(T), alignof(T));
    }

#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
    [[clang::always_inline]]
#endif
    constexpr void* any_allocate_impl(size_type n) noexcept {
        if (1u == n)
            return allocator().allocate_node(sizeof(T), alignof(T));

        return allocator().allocate_array(n, sizeof(T), alignof(T));
    }

#if SALT_HAS_ATTRIBUTE(ALWAYS_INLINE)
    [[clang::always_inline]]
#endif
    constexpr void any_deallocate_impl(void* ptr, size_type n) noexcept {
        if (1u == n)
            return allocator().deallocate_node(ptr, sizeof(T), alignof(T));

        return allocator().deallocate_array(ptr, n, sizeof(T), alignof(T));
    }

    template <typename U, raw_allocator OtherRawAllocator>
    friend class std_allocator_adapter;
    // clang-format on
};

template <typename T, typename U, typename Allocator>
constexpr bool operator==(std_allocator_adapter<T, Allocator> const& lhs,
                          std_allocator_adapter<U, Allocator> const& rhs) noexcept {
    using std_allocator = std_allocator_adapter<T, Allocator>;
    if constexpr (std_allocator::is_shared_allocator)
        return lhs.allocator() == rhs.allocator();
    else if constexpr (std_allocator::is_stateful_allocator)
        return &lhs.allocator() == &rhs.allocator();
    else
        return true;
}

// clang-format off
template <typename T>
using std_any_allocator = std_allocator_adapter<T, any_allocator>;
// clang-format on

} // namespace salt::memory