#pragma once

#include <salt/memory/allocator_storage.hpp>
#include <salt/memory/allocator_traits.hpp>
#include <salt/memory/threading.hpp>

namespace salt {

namespace detail {

// clang-format off
template <typename Allocator>
concept propagate_on_container_swap =
    requires {
       { typename Allocator::propagate_on_container_swap{} } -> std::same_as<std::true_type>;
    };

template <typename Allocator>
concept propagate_on_container_move_assignment =
    requires {
       { typename Allocator::propagate_on_container_move_assignment{} } -> std::same_as<std::true_type>;
    };

template <typename Allocator>
concept propagate_on_container_copy_assignment =
    requires {
       { typename Allocator::propagate_on_container_copy_assignment{} } -> std::same_as<std::true_type>;
    };

template <typename AllocatorReference>
concept any_reference = std::same_as<AllocatorReference, Any_allocator_reference>;
// clang-format on

} // namespace detail

// clang-format off
template <raw_allocator RawAllocator>
struct [[nodiscard]] propagation_traits final {
    using propagate_on_container_swap            =
            std::conditional_t<detail::propagate_on_container_swap<RawAllocator>, std::true_type,
                               std::false_type>;
    using propagate_on_container_move_assignment =
            std::conditional_t<detail::propagate_on_container_move_assignment<RawAllocator>,
                               std::true_type, std::false_type>;
    using propagate_on_container_copy_assignment =
            std::conditional_t<detail::propagate_on_container_copy_assignment<RawAllocator>,
                               std::true_type, std::false_type>;

    template <typename Allocator>
    static constexpr Allocator select_on_container_copy_construction(Allocator const& allocator) {
        return allocator;
    }
};
// clang-format on

template <typename Derived, typename Base>
concept not_derived_from = not std::derived_from<Derived, Base>;

template <typename T, raw_allocator RawAllocator>
class [[nodiscard]] Std_allocator : protected Allocator_reference<RawAllocator> {
    using allocator_reference = Allocator_reference<RawAllocator>;
    using allocator_traits    = allocator_traits<RawAllocator>;
    using propagation_traits  = propagation_traits<RawAllocator>;

    static constexpr bool is_any_reference      = detail::any_reference<allocator_reference>;
    static constexpr bool is_shared_allocator   = salt::is_shared_allocator<RawAllocator>::value;
    static constexpr bool is_stateful_allocator = allocator_traits::is_stateful::value;

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

    constexpr Std_allocator() noexcept requires(not is_stateful_allocator)
            : allocator_reference{allocator_type{}} {}

    constexpr explicit Std_allocator(allocator_reference const& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename Allocator> requires not_derived_from<Allocator, Std_allocator>
    constexpr Std_allocator(Allocator& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename Allocator> requires not_derived_from<Allocator, Std_allocator>
    constexpr Std_allocator(Allocator const& allocator) noexcept
            : allocator_reference{allocator} {}
    // clang-format on

    template <typename U>
    constexpr Std_allocator(Std_allocator<U, RawAllocator>& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename U>
    constexpr Std_allocator(Std_allocator<U, RawAllocator> const& allocator) noexcept
            : allocator_reference{allocator} {}

    // Implicit conversion from any other Allocator_storage is forbidden to prevent accidentally
    // wrapping another Allocator_storage inside a Allocator_reference.
    template <typename Storage, typename Mutex>
    constexpr Std_allocator(Allocator_storage<Storage, Mutex>&) = delete;

    [[nodiscard]] constexpr T* allocate(size_type n) {
        if constexpr (is_any_reference)
            return static_cast<T*>(any_allocate_impl(n));
        else
            return static_cast<T*>(allocate_impl(n));
    }

    constexpr void deallocate(T* ptr, size_type n) {
        if constexpr (is_any_reference)
            return any_deallocate_impl(ptr, n);
        else
            return deallocate_impl(ptr, n);
    }

    constexpr Std_allocator select_on_container_copy_construction() const {
        return propagation_traits::select_on_container_copy_construction(*this);
    }

    constexpr decltype(auto) allocator() noexcept {
        return allocator_reference::allocator();
    }
    constexpr decltype(auto) allocator() const noexcept {
        return allocator_reference::allocator();
    }

    template <typename T1, typename T2, typename Allocator>
    friend constexpr bool operator==(Std_allocator<T1, Allocator> const&,
                                     Std_allocator<T2, Allocator> const&) noexcept;

private:
    // clang-format off
#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
    [[msvc::forceinline]]
#endif
    constexpr void* allocate_impl(size_type n) {
        if (1u == n)
            return allocator_reference::allocate_node(sizeof(T), alignof(T));

        return allocator_reference::allocate_array(n, sizeof(T), alignof(T));
    }

#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
    [[msvc::forceinline]]
#endif
    constexpr void deallocate_impl(void* ptr, size_type n) {
        if (1u == n)
            return allocator_reference::deallocate_node(ptr, sizeof(T), alignof(T));

        return allocator_reference::deallocate_array(ptr, n, sizeof(T), alignof(T));
    }

#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
    [[msvc::forceinline]]
#endif
    constexpr void* any_allocate_impl(size_type n) {
        if (1u == n)
            return allocator().allocate_node(sizeof(T), alignof(T));

        return allocator().allocate_array(n, sizeof(T), alignof(T));
    }

#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
    [[msvc::forceinline]]
#endif
    constexpr void any_deallocate_impl(void* ptr, size_type n) {
        if (1u == n)
            return allocator().deallocate_node(ptr, sizeof(T), alignof(T));

        return allocator().deallocate_array(ptr, n, sizeof(T), alignof(T));
    }

    template <typename U, raw_allocator OtherRawAllocator>
    friend class Std_allocator;
    // clang-format on
};

template <typename T, typename U, typename Allocator>
constexpr bool operator==(Std_allocator<T, Allocator> const& lhs,
                          Std_allocator<U, Allocator> const& rhs) noexcept {
    using std_allocator = Std_allocator<T, Allocator>;
    if constexpr (std_allocator::is_shared_allocator)
        return lhs.allocator() == rhs.allocator();
    else if constexpr (std_allocator::is_stateful_allocator)
        return &lhs.allocator() == &rhs.allocator();
    else
        return true;
}

// clang-format off
template <typename T>
using Std_any_allocator = Std_allocator<T, Any_allocator>;

template <typename T, raw_allocator RawAllocator>
auto make_std_allocator(RawAllocator&& allocator) noexcept
        -> Std_allocator<T, typename std::decay_t<RawAllocator>>
{
    return {std::forward<RawAllocator>(allocator)};
}

template <typename T, raw_allocator RawAllocator>
auto make_std_any_allocator(RawAllocator&& allocator) noexcept
        -> Std_any_allocator<T>
{
    return {std::forward<RawAllocator>(allocator)};
}
// clang-format on

} // namespace salt