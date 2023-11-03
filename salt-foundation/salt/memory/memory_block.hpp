#pragma once
#include <cstddef>

namespace salt::memory {

struct [[nodiscard]] memory_block final {
    void*       memory;
    std::size_t size;

    constexpr memory_block() noexcept : memory_block(nullptr, std::size_t(0)) {}

    constexpr memory_block(void* begin, std::size_t n) noexcept : memory{begin}, size{n} {}

    constexpr memory_block(void* begin, void* end) noexcept
            : memory_block(begin, static_cast<std::size_t>(static_cast<std::byte*>(end) -
                                                           static_cast<std::byte*>(begin))) {}

    constexpr bool contains(void const* address) const noexcept {
        auto begin = static_cast<std::byte const*>(memory);
        auto ptr   = static_cast<std::byte const*>(address);
        return ptr >= begin && ptr < begin + size;
    }
};

} // namespace salt::memory