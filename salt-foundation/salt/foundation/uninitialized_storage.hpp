#pragma once
#include <salt/config.hpp>
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
            : value{std::move(other.value)} {}

    constexpr uninitialized_storage& operator=(uninitialized_storage const&) noexcept = delete;
    constexpr uninitialized_storage& operator=(uninitialized_storage&&) noexcept      = delete;

    constexpr ~uninitialized_storage() requires meta::trivially_destructible<T> = default;
    constexpr ~uninitialized_storage() {}
    // clang-format on

    [[nodiscard]] constexpr value_type* data() noexcept {
        return std::addressof(value);
    }
    [[nodiscard]] constexpr value_type const* data() const noexcept {
        return std::addressof(value);
    }
};

// clang-format off
template <typename T>
constexpr auto* data(uninitialized_storage<T>& storage) noexcept {
    return storage.data();
}
template <typename T>
constexpr auto const* data(uninitialized_storage<T> const& storage) noexcept {
    return storage.data();
}
template <typename T>
constexpr T* data(T& value) noexcept {
    return std::addressof(value);
}

template <typename T>
constexpr auto& get(uninitialized_storage<T>& storage) noexcept {
    return *storage.data();
}
template <typename T>
constexpr auto const& get(uninitialized_storage<T> const& storage) noexcept {
    return *storage.data();
}
template <typename T>
constexpr T&& get(T&& value) noexcept {
    return value;
}
// clang-format on

template <typename T>
using uninitialized_storage_for =
        meta::condition<meta::has_trivial_lifetime<T>, T, uninitialized_storage<T>>;

} // namespace salt::fdn