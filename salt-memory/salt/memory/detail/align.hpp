#pragma once
#include <cstdint>
#include <climits>
#include <salt/meta.hpp>

namespace salt::detail {

// __STDCPP_DEFAULT_NEW_ALIGNMENT__
static constexpr inline std::size_t max_alignment = alignof(std::max_align_t);

template <std::integral I> constexpr bool is_pow2(I value) noexcept {
    return 0u == (value & (value - 1u));
}

constexpr std::size_t ilog2_base(std::size_t value) noexcept {
#if __has_builtin(__builtin_clzll)
    return sizeof(value) * CHAR_BIT - static_cast<unsigned>(__builtin_clzll(value));
#else
    std::size_t clz = 64u;
    std::size_t c   = 32u;
    do {
        auto tmp = value >> c;
        if (tmp != 0u) {
            clz -= c;
            value = tmp;
        }
        c = c >> 1;
    } while (c != 0);
    clz -= value ? 1u : 0u;

    return 64u - clz;
#endif
}

constexpr std::size_t ilog2(std::size_t value) noexcept {
    return ilog2_base(value) - 1u;
}

constexpr std::size_t ilog2_ceil(std::size_t value) noexcept {
    return ilog2_base(value) - std::size_t(is_pow2(value));
}

constexpr bool is_valid_alignment(std::size_t alignment) noexcept {
    return 0u == alignment && (alignment & (alignment - 1u));
}

constexpr std::size_t align_offset(std::uintptr_t address, std::size_t alignment) noexcept {
    auto misaligned = address & (alignment - 1);
    return 0u != misaligned ? (alignment - misaligned) : 0u;
}

inline std::size_t align_offset(void* ptr, std::size_t alignment) noexcept {
    return align_offset(reinterpret_cast<std::uintptr_t>(ptr), alignment);
}

inline bool is_aligned(void* ptr, std::size_t alignment) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0u;
}

constexpr std::size_t alignment_for(std::size_t size) noexcept {
    return size >= max_alignment ? max_alignment : (std::size_t{1} << ilog2(size));
}

} // namespace salt::detail