#pragma once
#include <salt/config.hpp>
#include <salt/foundation/addressof.hpp>
#include <salt/foundation/array.hpp>
#include <salt/foundation/types.hpp>

namespace salt::fdn {

struct nontrivial_type {
    // This default constructor is user-provided to avoid
    // zero-initialization when objects are value-initialized.
    constexpr nontrivial_type() noexcept {}
};

template <typename T> union [[nodiscard]] uninitialized_storage final {
    using value_type = T;

    value_type      value;
    nontrivial_type _;

    constexpr uninitialized_storage() : _{} {}
    constexpr explicit uninitialized_storage(value_type const& v) noexcept : value{v} {}
    constexpr explicit uninitialized_storage(value_type&& v) noexcept : value{meta::move(v)} {}

    // clang-format off
    constexpr uninitialized_storage(uninitialized_storage const&) noexcept
        requires meta::trivially_copy_constructible<T> = default;
    constexpr uninitialized_storage(uninitialized_storage&&) noexcept
        requires meta::trivially_move_constructible<T> = default;

    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept
        requires meta::trivially_copy_assignable<T> = default;
    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept
        requires meta::trivially_move_assignable<T> = default;

    constexpr uninitialized_storage(uninitialized_storage const& other) noexcept
            : value{other.value} {}
    constexpr uninitialized_storage(uninitialized_storage&& other) noexcept
            : value{meta::move(other.value)} {}

    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept = delete;
    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept      = delete;

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

// clang-format off
template <meta::reference T, typename U>
constexpr auto& get(uninitialized_storage<U>& storage) noexcept {
    return *storage.data();
}
template <meta::reference T, typename U>
constexpr auto const& get(uninitialized_storage<U> const& storage) noexcept {
    return *storage.data();
}
template <meta::reference T, typename U>
constexpr U&& get(U&& value) noexcept {
    return value;
}

template <meta::pointer T, typename U>
constexpr U* get(uninitialized_storage<U>& storage) noexcept {
    return storage.data();
}
template <meta::pointer T, typename U>
constexpr U const* get(uninitialized_storage<U> const& storage) noexcept {
    return storage.data();
}
template <meta::pointer T, typename U>
constexpr U* get(U& value) noexcept {
    return addressof(value);
}
// clang-format on


// Optional for types with trivial lifetimes, i.e. such types will
// not be wrapped by an uninitialized_storage.
template <typename T>
using optional_uninitialized_storage =
        meta::condition<meta::has_trivial_lifetime<T>, T, uninitialized_storage<T>>;

} // namespace salt::fdn