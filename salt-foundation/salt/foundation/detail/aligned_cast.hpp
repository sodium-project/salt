#pragma once
#include <memory>
#include <new>

namespace salt::detail {

#if SALT_TARGET_WINDOWS
using std::assume_aligned;
// Since Apple put a big willie on the implementation of features from C++20, I have to implement
// everything instead of them. What could be better than that?
#elif SALT_TARGET_MACOSX && SALT_CLANG_FULL_VER <= 130106
template <std::size_t N, typename T> [[nodiscard]] constexpr T* assume_aligned(T* ptr) {
    static_assert(N != 0 && (N & (N - 1)) == 0,
                  "std::assume_aligned<N>(p) requires N to be a power of two");

    return static_cast<T*>(__builtin_assume_aligned(ptr, N));
}
#endif

template <typename To, typename From> auto aligned_cast(From const& x) noexcept {
    return assume_aligned<alignof(salt::remove_all_pointers_t<To>)>(
#if __cplusplus < 202002L
            std::launder(reinterpret_cast<To>(x))
#else
            reinterpret_cast<To>(x)
#endif
    );
}

} // namespace salt::detail