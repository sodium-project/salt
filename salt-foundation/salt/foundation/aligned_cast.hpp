#pragma once
#include <salt/meta.hpp>

namespace salt::fdn {

template <std::integral I>
constexpr bool is_pow2(I value) noexcept {
    return value && !((value) & (value - 1));
}

template <typename T>
concept aligned_as_pow2 = requires {
    requires is_pow2(alignof(T));
};

template <std::size_t N, typename T>
[[nodiscard]] constexpr T* assume_aligned(T* ptr) {
    static_assert(N != 0 && (N & (N - 1)) == 0,
                  "assume_aligned<N>(p) requires N to be a power of two");
#if __has_builtin(__builtin_assume_aligned)
    if consteval {
        return ptr;
    } else {
        return static_cast<T*>(__builtin_assume_aligned(ptr, N));
    }
#else
    return ptr;
#endif
}

template <typename To, typename From>
[[nodiscard]] constexpr To aligned_cast(From const& x) noexcept {
    return assume_aligned<alignof(meta::remove_all_pointers_t<To>)>(
#if __cplusplus < 202002L
            std::launder(reinterpret_cast<To>(x))
#else
            reinterpret_cast<To>(x)
#endif
    );
}

} // namespace salt::fdn