#pragma once
#include <memory>
#include <new>

namespace salt::detail {

template <typename To, typename From> auto aligned_cast(From const& x) noexcept {
    return std::assume_aligned<alignof(salt::remove_all_pointers_t<To>)>(
#if __cplusplus < 202002L
            std::launder(reinterpret_cast<To>(x))
#else
            reinterpret_cast<To>(x)
#endif
    );
}

} // namespace salt::detail