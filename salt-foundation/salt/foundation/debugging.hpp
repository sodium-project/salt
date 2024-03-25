#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace salt::fdn {

// The magic values that are used for debug filling. If SALT_MEMORY_DEBUG_FILL is true, memory will
// be filled to help detect use-after-free or missing initialization errors. These are the
// constants for the different types.
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

// Contains information about an allocator. It can be used for logging in the various handler
// functions.
struct [[nodiscard]] allocator_info final {
    std::string_view name;
    void const*      allocator;

    friend constexpr bool operator==(allocator_info const& lhs,
                                     allocator_info const& rhs) noexcept = default;
};

// The type of the handler called when a memory leak is detected.
using leak_handler = void (*)(allocator_info const& info, std::ptrdiff_t amount);

leak_handler set_leak_handler(leak_handler handler);

leak_handler get_leak_handler();

// The type of the handler called when an invalid pointer is passed to a deallocation function.
using invalid_pointer_handler = void (*)(allocator_info const& info, void const* ptr);

invalid_pointer_handler set_invalid_pointer_handler(invalid_pointer_handler handler);

invalid_pointer_handler get_invalid_pointer_handler();

// The type of the handler called when a buffer under/overflow is detected.
using buffer_overflow_handler = void (*)(void const* memory, std::size_t size, void const* ptr);

buffer_overflow_handler set_buffer_overflow_handler(buffer_overflow_handler handler);

buffer_overflow_handler get_buffer_overflow_handler();

} // namespace salt::fdn