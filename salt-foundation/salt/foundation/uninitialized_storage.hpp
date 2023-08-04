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

template <typename T>
using transparent_uninitialized_storage =
        meta::condition<meta::has_trivial_lifetime<T>, T, uninitialized_storage<T>>;

} // namespace salt::fdn