#pragma once
#include <salt/foundation/detail/aligned_cast.hpp>

namespace salt {

using std::constructible_from;
using std::destructible;

template <std::size_t Size, std::size_t Alignment>
struct [[nodiscard]] Uninitialized_storage final {
    // clang-format off
    template <typename T, typename... Args>
    requires sized             <T, Size     >
          && aligned           <T, Alignment>
          && constructible_from<T, Args...  >
    T& construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        auto const storage = get<T>();
        ::new (storage) T{std::forward<Args>(args)...};
        return *storage;
    }
    // clang-format on

    template <destructible T> void destruct() noexcept {
        get<T>()->~T();
    }

    template <aligned_as_pow2 T> [[nodiscard]] T* get() noexcept {
        return detail::aligned_cast<T*>(addressof(storage_));
    }

    template <aligned_as_pow2 T> [[nodiscard]] T const* get() const noexcept {
        return detail::aligned_cast<T const*>(addressof(storage_));
    }

private:
    alignas(Alignment) std::byte storage_[Size];
};

template <typename T>
using Uninitialized_storage_for = Uninitialized_storage<sizeof(T), alignof(T)>;

} // namespace salt