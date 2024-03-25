#pragma once
#include <salt/foundation/align.hpp>
#include <salt/foundation/detail/debug_helpers.hpp>

namespace salt::fdn::detail {

// clang-format off
template <typename Allocator>
struct [[nodiscard]] low_level_allocator_leak_handler {
    constexpr void operator()(std::ptrdiff_t amount) noexcept {
        debug_handle_memory_leak(Allocator::info(), amount);
    }
};

template <typename Allocator>
concept low_level_allocator =
    requires(std::size_t size, std::size_t align) {
        // A low-level allocator should have static allocate/deallocate functions...
        { Allocator::allocate  (         size, align) } noexcept -> meta::same_as<void*>;
        { Allocator::deallocate(nullptr, size, align) } noexcept -> meta::same_as<void>;
        // ...as well as auxiliary utilities.
        { Allocator::max_size() } noexcept -> meta::same_as<std::size_t>;
        { Allocator::info()     } noexcept -> meta::same_as<allocator_info>;
    };
// clang-format on

template <low_level_allocator Allocator>
struct [[nodiscard]] low_level_allocator_adapter
        : global_leak_detector<low_level_allocator_leak_handler<Allocator>> {
    using allocator_type  = Allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using leak_detector   = global_leak_detector<low_level_allocator_leak_handler<Allocator>>;
    using stateful        = meta::false_type;

    constexpr low_level_allocator_adapter() noexcept = default;
    constexpr ~low_level_allocator_adapter()         = default;

    constexpr low_level_allocator_adapter(low_level_allocator_adapter&&) noexcept            = default;
    constexpr low_level_allocator_adapter& operator=(low_level_allocator_adapter&&) noexcept = default;

    constexpr void* allocate_node(size_type size, size_type alignment) noexcept {
        auto actual_size = size + (debug_fence_size ? 2u * max_alignment : 0u);
        auto memory      = allocator_type::allocate(actual_size, alignment);

        leak_detector::on_allocate(actual_size);

        return debug_fill_new(memory, size, max_alignment);
    }

    constexpr void deallocate_node(void* node, size_type size, size_type alignment) noexcept {
        auto actual_size = size + (debug_fence_size ? 2u * max_alignment : 0u);
        auto memory      = debug_fill_free(node, size, max_alignment);

        allocator_type::deallocate(memory, actual_size, alignment);

        leak_detector::on_deallocate(actual_size);
    }

    constexpr size_type max_node_size() const noexcept {
        return allocator_type::max_size();
    }
};

#define SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(allocator, name)                                     \
    SALT_MEMORY_GLOBAL_LEAK_DETECTOR(low_level_allocator_leak_handler<allocator>, name)

} // namespace salt::fdn::detail