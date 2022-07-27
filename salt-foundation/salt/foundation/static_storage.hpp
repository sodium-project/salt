#pragma once
#include <type_traits>

namespace salt {

struct [[nodiscard]] In_place final {
    explicit In_place() = default;
};
inline constexpr In_place in_place{};

template <typename T, std::size_t Size, std::size_t Alignment>
struct [[nodiscard]] Static_storage final {
    // clang-format off
    template <typename... Args> requires constructible_from<T, Args...>
    explicit Static_storage(In_place,
                            Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        storage_.template construct<T>(std::forward<Args>(args)...);
    }
    // clang-format on

    ~Static_storage() {
        storage_.template destruct<T>();
    }

    Static_storage(Static_storage const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        storage_.template construct<T>(*other);
    }

    Static_storage(Static_storage&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        storage_.template construct<T>(std::move(*other));
    }

    Static_storage&
    operator=(Static_storage const& other) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        if (this != &other) {
            // clang-format off
            storage_.template destruct <T>();
            storage_.template construct<T>(*other);
            // clang-format on
        }
        return *this;
    }

    Static_storage&
    operator=(Static_storage&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (this != &other) {
            // clang-format off
            storage_.template destruct <T>();
            storage_.template construct<T>(std::move(*other));
            // clang-format on
        }
        return *this;
    }

    // clang-format off
    auto get()       noexcept { return storage_.template get<T>(); }
    auto get() const noexcept { return storage_.template get<T>(); }
    // clang-format on

    // clang-format off
    T* operator->() noexcept { return  get(); }
    T& operator* () noexcept { return *get(); }

    T const* operator->() const noexcept { return  get(); }
    T const& operator* () const noexcept { return *get(); }
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