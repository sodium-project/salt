#pragma once
#include <cstddef>

namespace salt {

struct [[nodiscard]] Memory_block final {
    void*       memory;
    std::size_t size;

    constexpr Memory_block() noexcept : Memory_block(nullptr, std::size_t(0)) {}

    constexpr Memory_block(void* begin, std::size_t n) noexcept : memory{begin}, size{n} {}

    constexpr Memory_block(void* begin, void* end) noexcept
            : Memory_block(begin, static_cast<std::size_t>(static_cast<std::byte*>(end) -
                                                           static_cast<std::byte*>(begin))) {}

    constexpr bool contains(void const* address) const noexcept {
        auto begin = static_cast<std::byte const*>(memory);
        auto ptr   = static_cast<std::byte const*>(address);
        return ptr >= begin && ptr < begin + size;
    }
};

} // namespace salt
