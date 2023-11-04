#include <salt/foundation/memory/detail/debug_helpers.hpp>

#include <salt/foundation/memory/debugging.hpp>

namespace salt::memory::detail {

#if SALT_MEMORY_DEBUG_FILL
void debug_fill(void* memory, std::size_t size, debug_magic magic_value) noexcept {
#    if __has_builtin(__builtin_memset)
    __builtin_memset(memory, static_cast<int>(magic_value), size);
#    else
    std::memset(memory, static_cast<int>(magic_value), size);
#    endif
}

void* debug_is_filled(void* memory, std::size_t size, debug_magic magic_value) noexcept {
    auto byte = static_cast<std::byte*>(memory);
    for (auto const end = byte + size; byte != end; ++byte) {
        if (*byte != static_cast<std::byte>(magic_value))
            return byte;
    }
    return nullptr;
}

void* debug_fill_new(void* memory, std::size_t node_size, std::size_t fence_size) noexcept {
    if (!debug_fence_size)
        fence_size = 0u;

    auto mem = static_cast<std::byte*>(memory);
    debug_fill(mem, fence_size, debug_magic::fence_memory);

    mem += fence_size;
    debug_fill(mem, node_size, debug_magic::new_memory);

    debug_fill(mem + node_size, fence_size, debug_magic::fence_memory);

    return mem;
}

void* debug_fill_free(void* memory, std::size_t node_size, std::size_t fence_size) noexcept {
    if (!debug_fence_size)
        fence_size = 0u;

    debug_fill(memory, node_size, debug_magic::freed_memory);

    auto pre_fence = static_cast<std::byte*>(memory) - fence_size;
    if (auto pre_dirty = debug_is_filled(pre_fence, fence_size, debug_magic::fence_memory))
        get_buffer_overflow_handler()(memory, node_size, pre_dirty);

    auto post_mem = static_cast<std::byte*>(memory) + node_size;
    if (auto post_dirty = debug_is_filled(post_mem, fence_size, debug_magic::fence_memory))
        get_buffer_overflow_handler()(memory, node_size, post_dirty);

    return pre_fence;
}

void debug_fill_internal(void* memory, std::size_t size, bool free) noexcept {
    debug_fill(memory, size,
               free ? debug_magic::internal_freed_memory : debug_magic::internal_memory);
}
#endif // SALT_MEMORY_DEBUG_FILL

void debug_handle_invalid_ptr(allocator_info const& info, void* ptr) noexcept {
    get_invalid_pointer_handler()(info, ptr);
}

void debug_handle_memory_leak(allocator_info const& info, std::ptrdiff_t amount) noexcept {
    get_leak_handler()(info, amount);
}

} // namespace salt::memory::detail