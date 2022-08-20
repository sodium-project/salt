#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace salt {

enum class debug_magic : std::uint8_t {
    // Marks internal memory used by the allocator - "allocated block".
    internal_memory = 0xAB,
    // Marks internal memory currently not used by the allocator - "freed block".
    internal_freed_memory = 0xFB,
    // Marks allocated, but not yet used memory - "clean memory".
    new_memory = 0xCD,
    // Marks freed memory - "dead memory".
    freed_memory = 0xDD,
    // Marks buffer memory used to ensure proper alignment.
    alignment_memory = 0xED,
    // Marks buffer memory used to protect against overflow - "fence memory".
    fence_memory = 0xFD
};

struct [[nodiscard]] Allocator_info final {
    std::string_view name;
    void const*      allocator;

    friend constexpr bool operator==(Allocator_info const& lhs,
                                     Allocator_info const& rhs) noexcept = default;
};

using leak_handler = void (*)(Allocator_info const& info, std::ptrdiff_t amount);

leak_handler set_leak_handler(leak_handler handler);

leak_handler get_leak_handler();

using invalid_pointer_handler = void (*)(Allocator_info const& info, void const* ptr);

invalid_pointer_handler set_invalid_pointer_handler(invalid_pointer_handler handler);

invalid_pointer_handler get_invalid_pointer_handler();

using buffer_overflow_handler = void (*)(void const* memory, std::size_t size, void const* ptr);

buffer_overflow_handler set_buffer_overflow_handler(buffer_overflow_handler handler);

buffer_overflow_handler get_buffer_overflow_handler();

} // namespace salt
