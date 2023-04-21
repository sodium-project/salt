#pragma once
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/debug_helpers.hpp>

namespace salt::detail {

template <typename Allocator> struct Low_level_allocator_leak_handler {
    void operator()(std::ptrdiff_t amount) {
        debug_handle_memory_leak(Allocator::info(), amount);
    }
};

// clang-format off
template <typename Allocator, std::size_t Size = 1, std::size_t Alignment = 1>
concept allocator_like =
    requires {
        { Allocator::allocate(Size, Alignment)            } -> std::same_as<void*>;
        { Allocator::deallocate(nullptr, Size, Alignment) } -> std::same_as<void>;
        { Allocator::max_size()                           } -> std::same_as<std::size_t>;
        { Allocator::info()                               } -> std::same_as<Allocator_info>;
    };
// clang-format on

template <allocator_like Allocator>
struct [[nodiscard]] Low_level_allocator
        : Global_leak_detector<Low_level_allocator_leak_handler<Allocator>> {
    using is_stateful     = std::false_type;
    using leak_detector   = Global_leak_detector<Low_level_allocator_leak_handler<Allocator>>;
    using allocator_type  = Allocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    Low_level_allocator() noexcept = default;
    ~Low_level_allocator()         = default;

    Low_level_allocator(Low_level_allocator&&) noexcept            = default;
    Low_level_allocator& operator=(Low_level_allocator&&) noexcept = default;

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
    SALT_MEMORY_GLOBAL_LEAK_DETECTOR(Low_level_allocator_leak_handler<allocator>, name)

} // namespace salt::detail
