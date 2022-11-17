#pragma once
#include <memory>
#include <new>

namespace salt::detail {

#if defined(__cpp_lib_assume_aligned)
using std::assume_aligned;
#else
template <std::size_t N, typename T> [[nodiscard]] constexpr T* assume_aligned(T* ptr) {
    static_assert(N != 0 && (N & (N - 1)) == 0, "assume_aligned<N>(p) requires N to be a power of two");
#    if __has_builtin(__builtin_assume_aligned)
    if consteval {
        return ptr;
    } else {
        return static_cast<T*>(__builtin_assume_aligned(ptr, N));
    }
#    else
    return ptr;
#    endif
}
#endif

template <typename To, typename From> constexpr To aligned_cast(From const& x) noexcept {
    return assume_aligned<alignof(salt::remove_all_pointers_t<To>)>(
#if __cplusplus < 202002L
            std::launder(reinterpret_cast<To>(x))
#else
            reinterpret_cast<To>(x)
#endif
    );
}

} // namespace salt::detail