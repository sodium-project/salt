#pragma once
#include <type_traits>

namespace salt {

template <typename T, std::size_t Size, std::size_t Alignment>
struct [[nodiscard]] Static_storage final {
    // clang-format off
    template <typename... Args> requires std::constructible_from<T, Args...>
    constexpr explicit Static_storage(std::in_place_t, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>
    ) {
        storage_.template construct<T>(std::forward<Args>(args)...);
    }

    constexpr ~Static_storage() {
        storage_.template destruct<T>();
    }

    constexpr
    Static_storage(Static_storage const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        storage_.template construct<T>(*other);
    }

    constexpr
    Static_storage(Static_storage&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        storage_.template construct<T>(std::move(*other));
    }
    // clang-format on

    constexpr Static_storage&
    operator=(Static_storage const& other) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        if (this != std::addressof(other)) {
            // clang-format off
            storage_.template destruct <T>();
            storage_.template construct<T>(*other);
            // clang-format on
        }
        return *this;
    }

    constexpr Static_storage&
    operator=(Static_storage&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (this != std::addressof(other)) {
            // clang-format off
            storage_.template destruct <T>();
            storage_.template construct<T>(std::move(*other));
            // clang-format on
        }
        return *this;
    }

    // clang-format off
    constexpr auto get()       noexcept { return storage_.template get<T>(); }
    constexpr auto get() const noexcept { return storage_.template get<T>(); }
    // clang-format on

    // clang-format off
    constexpr T* operator->() noexcept { return  get(); }
    constexpr T& operator* () noexcept { return *get(); }

    constexpr T const* operator->() const noexcept { return  get(); }
    constexpr T const& operator* () const noexcept { return *get(); }
    // clang-format on

private:
    SALT_NO_UNIQUE_ADDRESS
    Uninitialized_storage<Size, Alignment> storage_;
};

template <typename T, std::size_t Size, std::size_t Alignment>
static inline bool operator==(Static_storage<T, Size, Alignment> const& lhs,
                              Static_storage<T, Size, Alignment> const& rhs) noexcept {
    return *lhs == *rhs;
}

template <typename T> using Static_storage_for = Static_storage<T, sizeof(T), alignof(T)>;

} // namespace salt