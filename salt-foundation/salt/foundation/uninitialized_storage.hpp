#pragma once
#include <array>
#include <memory>

#include <salt/foundation/detail/aligned_cast.hpp>

namespace salt {

template <std::size_t Size, std::size_t Alignment>
struct [[nodiscard]] Uninitialized_storage final {
    // clang-format off
    template <typename T, typename... Args> requires
        match_size             <T, Size>      and
        match_alignment        <T, Alignment> and
        std::constructible_from<T, Args...>
    constexpr T& construct(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>
    ) {
        auto const storage = get<T>();
        ::new (storage) T{std::forward<Args>(args)...};
        return *storage;
    }
    // clang-format on

    template <std::destructible T> constexpr void destruct() noexcept {
        get<T>()->~T();
    }

    template <aligned_as_pow2 T> [[nodiscard]] constexpr T* get() noexcept {
        return detail::aligned_cast<T*>(&storage_);
    }

    template <aligned_as_pow2 T> [[nodiscard]] constexpr T const* get() const noexcept {
        return detail::aligned_cast<T const*>(&storage_);
    }

private:
    SALT_NO_UNIQUE_ADDRESS
    alignas(Alignment) std::byte storage_[Size];
};

template <typename T>
using Uninitialized_storage_for = Uninitialized_storage<sizeof(T), alignof(T)>;

} // namespace salt