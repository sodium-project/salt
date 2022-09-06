#pragma once
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/debug_helpers.hpp>

#include <salt/memory/memory_arena.hpp>
#include <salt/memory/memory_pool_type.hpp>

namespace salt {

namespace detail {
struct Memory_pool_leak_handler {
    void operator()(std::ptrdiff_t amount) {
        get_leak_handler()({"salt::Memory_pool", this}, amount);
    }
};
} // namespace detail

// NOTE:
//  It uses a memory_arena with a given BlockOrRawAllocator defaulting to Growing_block_allocator,
//  subdivides them in small nodes of given size and puts them onto a free list. Allocation and
//  deallocation simply remove or add nodes from this list and are thus fast. The way the list is
//  maintained can be controlled via the PoolType which is either Node_pool, Array_pool. This kind
//  of allocator is ideal for fixed size allocations and deallocations in any order, for example in
//  a node based container like std::list. It is not so good for different allocation sizes and has
//  some drawbacks for arrays.
// clang-format off
template <
    typename PoolType            = Node_pool,
    typename BlockOrRawAllocator = Default_allocator,
    bool     Cached              = disable_caching
>
// clang-format on
class [[nodiscard]] Memory_pool : detail::Default_leak_detector<detail::Memory_pool_leak_handler> {
    using free_list          = typename PoolType::type;
    using memory_block_stack = detail::Memory_block_stack;
    using leak_detector      = detail::Default_leak_detector<detail::Memory_pool_leak_handler>;

public:
    using allocator_type  = block_allocator_type<BlockOrRawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using pool_type       = PoolType;

    template <typename... Args>
    constexpr Memory_pool(size_type node_size, size_type block_size, Args&&... args)
            : memory_arena_{block_size, std::forward<Args>(args)...}, free_list_{node_size} {
        allocate_block();
    }

    constexpr ~Memory_pool() {}

    constexpr Memory_pool(Memory_pool&& other) noexcept
            : leak_detector{std::move(other)}, memory_arena_{std::move(other.memory_arena_)},
              free_list_{std::move(other.free_list_)} {}

    constexpr Memory_pool& operator=(Memory_pool&& other) noexcept = default;

    constexpr void* allocate_node() {
        if (free_list_.empty()) [[unlikely]]
            allocate_block();
        return free_list_.allocate();
    }

    constexpr void* try_allocate_node() noexcept {
        return free_list_.empty() ? nullptr : free_list_.allocate();
    }

    constexpr void* allocate_array(size_type count) {
        return allocate_array(count, node_size());
    }

    constexpr void* try_allocate_array(size_type count) noexcept {
        return try_allocate_array(count, node_size());
    }

    constexpr void deallocate_node(void* node) noexcept {
        free_list_.deallocate(node);
    }

    constexpr bool try_deallocate_node(void* node) noexcept {
        if (!memory_arena_.contains(node)) [[unlikely]]
            return false;
        free_list_.deallocate(node);
        return true;
    }

    constexpr void deallocate_array(void* ptr, size_type count) noexcept {
        free_list_.deallocate(ptr, count * node_size());
    }

    constexpr bool try_deallocate_array(void* ptr, size_type count) noexcept {
        return try_deallocate_array(ptr, count, node_size());
    }

    constexpr size_type node_size() const noexcept {
        return free_list_.node_size();
    }

    constexpr size_type size() const noexcept {
        return free_list_.usable_size(memory_arena_.next_block_size());
    }

    constexpr size_type capacity() const noexcept {
        return free_list_.capacity() * node_size();
    }

    constexpr allocator_type& allocator() noexcept {
        return memory_arena_.allocator();
    }

    static constexpr size_type min_node_size = free_list::min_element_size;

    static constexpr size_type min_block_size(size_type node_size, size_type count) noexcept {
        return memory_block_stack::offset() + free_list::min_block_size(node_size, count);
    }

private:
    friend allocator_traits<Memory_pool>;

    constexpr auto info() const noexcept {
        return Allocator_info{"salt::memory_pool", this};
    }

    constexpr void allocate_block() {
        auto block = memory_arena_.allocate_block();
        free_list_.insert(static_cast<std::byte*>(block.memory), block.size);
    }

    constexpr void* allocate_array(size_type count, size_type node_size) {
        auto* memory = free_list_.empty() ? nullptr : free_list_.allocate(count * node_size);
        if (!memory) {
            allocate_block();
            memory = free_list_.allocate(count * node_size);
            if (!memory) [[unlikely]]
                throw std::bad_alloc();
        }
        return memory;
    }

    constexpr void* try_allocate_array(size_type count, size_type node_size) noexcept {
        return free_list_.empty() ? nullptr : free_list_.allocate(count * node_size);
    }

    constexpr bool try_deallocate_array(void* ptr, size_type count, size_type node_size) noexcept {
        if (!memory_arena_.contains(ptr))
            return false;
        free_list_.deallocate(ptr, count * node_size);
        return true;
    }

    Memory_arena<allocator_type, Cached> memory_arena_;
    free_list                            free_list_;
};

template <typename PoolType, typename RawAllocator, bool Cached>
struct [[nodiscard]] allocator_traits<Memory_pool<PoolType, RawAllocator, Cached>> final {
    using allocator_type  = Memory_pool<PoolType, RawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     = std::true_type;

    // clang-format off
    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment)
    {
        (void)alignment;
        auto* memory = allocator.allocate_node();
        allocator.on_allocate(size);
        return memory;
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment)
    {
        (void)alignment;
        auto* memory = allocator.allocate_array(count, size);
        allocator.on_allocate(count * size);
        return memory;
    }

    static constexpr void
    deallocate_node(allocator_type& allocator,
                    void*           node     ,
                    size_type       size     ,
                    size_type       alignment) noexcept
    {
        (void)alignment;
        allocator.deallocate_node(node);
        allocator.on_deallocate(size);
    }

    static constexpr void
    deallocate_array(allocator_type& allocator,
                     void*           array    ,
                     size_type       count    ,
                     size_type       size     ,
                     size_type       alignment) noexcept
    {
        (void)alignment;
        allocator.free_list_.deallocate(array, count * size);
        allocator.on_deallocate(count * size);
    }
    // clang-format on

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        return allocator.node_size();
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        return allocator.size();
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        return allocator.free_list_.alignment();
    }
};

} // namespace salt