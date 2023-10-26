#pragma once
#include <climits>
#include <cstddef>
#include <cstdint>
#include <salt/meta.hpp>

namespace salt::memory {

// clang-format off
template <meta::integral I>
constexpr bool is_pow2(I value) noexcept {
    return value && !((value) & (value - 1));
}
// clang-format on

namespace detail {

constexpr auto ilog2_base(unsigned long long value) noexcept {
#if __has_builtin(__builtin_clzll)
    return sizeof(value) * CHAR_BIT - static_cast<unsigned>(__builtin_clzll(value));
#else
    auto clz = 64ull;
    auto c   = 32ull;
    do {
        auto tmp = value >> c;
        if (tmp != 0ull) {
            clz -= c;
            value = tmp;
        }
        c = c >> 1ull;
    } while (c != 0ull);
    clz -= value ? 1ull : 0ull;

    return 64ull - clz;
#endif
}

} // namespace detail

constexpr std::size_t ilog2(std::size_t value) noexcept {
    return ilog2_base(value) - 1ull;
}

constexpr std::size_t ilog2_ceil(std::size_t value) noexcept {
    return ilog2_base(value) - static_cast<std::size_t>(is_pow2(value));
}

constexpr std::size_t alignment_for(std::size_t size) noexcept {
    return size >= max_alignment ? max_alignment : (std::size_t{1} << ilog2(size));
}

constexpr std::size_t align_offset(std::uintptr_t address, std::size_t alignment) noexcept {
    static_assert(is_pow2(alignment));
    auto misaligned = address & (alignment - 1);
    return 0ull != misaligned ? (alignment - misaligned) : 0ull;
}

inline std::size_t align_offset(void* ptr, std::size_t alignment) noexcept {
    return align_offset(reinterpret_cast<std::uintptr_t>(ptr), alignment);
}

inline bool is_aligned(void* ptr, std::size_t alignment) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0ull;
}

} // namespace salt::memory