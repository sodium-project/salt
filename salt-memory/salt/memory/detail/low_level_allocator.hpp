#pragma once
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/debug_helpers.hpp>

namespace salt::detail {

template <typename Allocator> struct Low_level_allocator_leak_handler {
    void operator()(std::ptrdiff_t amount) {
        (void)amount;
        debug_handle_memory_leak(Allocator::info(), amount);
    }
};

// TODO: Add concept for `Allocator` contraints;
template <typename Allocator>
struct [[nodiscard]] Low_level_allocator
        : Global_leak_detector<Low_level_allocator_leak_handler<Allocator>> {
    using is_stateful        = std::false_type;
    using allocator_type     = Allocator;
    using leak_detector_type = Global_leak_detector<Low_level_allocator_leak_handler<Allocator>>;

    constexpr Low_level_allocator() noexcept  = default;
    constexpr ~Low_level_allocator() noexcept = default;

    constexpr Low_level_allocator(Low_level_allocator&&) noexcept            = default;
    constexpr Low_level_allocator& operator=(Low_level_allocator&&) noexcept = default;

    constexpr void* allocate_node(std::size_t size, std::size_t alignment) noexcept {
        auto const actual_size = size + (debug_fence_size ? 2u * max_alignment : 0u);

        auto memory = allocator_type::allocate(actual_size, alignment);
        // if (!memory)
        //     salt::error("Out of memory"); // allocator_type::info(), actual_size

        leak_detector_type::on_allocate(static_cast<std::uint32_t>(actual_size));

        return debug_fill_new(memory, size, max_alignment);
    }

    constexpr void deallocate_node(void* node, std::size_t size, std::size_t alignment) noexcept {
        auto const actual_size = size + (debug_fence_size ? 2u * max_alignment : 0u);

        auto memory = debug_fill_free(node, size, max_alignment);
        allocator_type::deallocate(memory, actual_size, alignment);

        leak_detector_type::on_deallocate(static_cast<std::uint32_t>(actual_size));
    }

    constexpr std::size_t max_node_size() const noexcept {
        return allocator_type::max_node_size();
    }
};

#define SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(allocator, name)                                     \
    SALT_MEMORY_GLOBAL_LEAK_DETECTOR(Low_level_allocator_leak_handler<allocator>, name)

} // namespace salt::detail
