#pragma once
#include <salt/config.hpp>
#include <salt/meta.hpp>

#include <salt/memory/addressof.hpp>

namespace salt::memory {

struct nontrivial_type {
    // This default constructor is user-provided to avoid
    // zero-initialization when objects are value-initialized.
    constexpr nontrivial_type() noexcept {}
};

template <typename T>
union [[nodiscard, clang::trivial_abi]] uninitialized_storage final {
    using value_type = T;

    value_type      value;
    nontrivial_type _;

    constexpr uninitialized_storage() : _{} {}

    constexpr uninitialized_storage(uninitialized_storage const& other) noexcept
        requires meta::trivially_copy_constructible<T>
    = default;
    constexpr uninitialized_storage(uninitialized_storage const& other) noexcept
            : value{other.value} {}

    constexpr uninitialized_storage(uninitialized_storage&& other) noexcept
        requires meta::trivially_move_constructible<T>
    = default;
    constexpr uninitialized_storage(uninitialized_storage&& other) noexcept
            : value{meta::move(other.value)} {}

    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept
        requires meta::trivially_copy_assignable<T>
    = default;
    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept = delete;

    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept
        requires meta::trivially_move_assignable<T>
    = default;
    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept = delete;

    // clang-format off
    constexpr ~uninitialized_storage() requires meta::trivially_destructible<T> = default;
    constexpr ~uninitialized_storage() {}
    // clang-format on

    [[nodiscard]] constexpr value_type* data() noexcept {
        return addressof(value);
    }
    [[nodiscard]] constexpr value_type const* data() const noexcept {
        return addressof(value);
    }
};

template <typename T>
constexpr bool operator==(uninitialized_storage<T> const& lhs,
                          uninitialized_storage<T> const& rhs) noexcept {
    return lhs.value == rhs.value;
}

} // namespace salt::memory

namespace salt {

// clang-format off
template <meta::reference Reference, typename T>
constexpr Reference get(memory::uninitialized_storage<T>& storage) noexcept {
    return *storage.data();
}
template <meta::reference Reference, typename T>
constexpr Reference get(memory::uninitialized_storage<T> const& storage) noexcept {
    return *storage.data();
}
template <meta::reference Reference, typename T>
constexpr T&& get(T&& value) noexcept {
    return value;
}

template <meta::pointer Pointer, typename T>
constexpr Pointer get(memory::uninitialized_storage<T>& storage) noexcept {
    return storage.data();
}
template <meta::pointer Pointer, typename T>
constexpr Pointer get(memory::uninitialized_storage<T> const& storage) noexcept {
    return storage.data();
}
template <meta::pointer Pointer, typename T>
constexpr T* get(T& value) noexcept {
    return memory::addressof(value);
}
// clang-format on

// Optional, since types with trivial lifetimes will not be wrapped by an uninitialized storage.
template <typename T>
using optional_uninitialized_storage =
        meta::condition<meta::has_trivial_lifetime<T>, T, memory::uninitialized_storage<T>>;

} // namespace salt