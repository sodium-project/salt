#pragma once
#include <salt/foundation/memory/align.hpp>
#include <salt/foundation/memory/memory_arena.hpp>
#include <salt/foundation/memory/memory_pool_type.hpp>

namespace salt::memory {

// It uses a `memory_arena` with a given `BlockOrRawAllocator` defaulted to `growing_block_allocator`,
// subdivides them in small nodes of given size and puts them onto a free list. Allocation and
// deallocation simply remove or add nodes from this list and are thus fast. The way the list is
// maintained can be controlled via the `PoolType` which is either `node_pool` or `array_pool`. This
// kind of allocator is ideal for fixed size allocations and deallocations in any order, for example
// in a node based container like `std::list`. It is not so good for different allocation sizes and has
// some drawbacks for `array`s.
// clang-format off
template <
    typename PoolType            = node_pool,
    typename BlockOrRawAllocator = default_allocator,
    bool     IsCached            = disable_caching
>
// clang-format on
class [[nodiscard]] memory_pool : default_leak_detector<detail::memory_pool_leak_handler> {
    using memory_list        = typename PoolType::type;
    using memory_block_stack = detail::memory_block_stack;
    using leak_detector      = default_leak_detector<detail::memory_pool_leak_handler>;

public:
    using allocator_type  = block_allocator_type<BlockOrRawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using pool_type       = PoolType;

    template <typename... Args>
    constexpr memory_pool(size_type node_size, size_type block_size, Args&&... args) noexcept
            : arena_{block_size, meta::forward<Args>(args)...}, list_{node_size} {
        allocate_block();
    }

    constexpr ~memory_pool() = default;

    constexpr memory_pool(memory_pool&& other) noexcept
            : leak_detector{meta::move(other)       },
              arena_       {meta::move(other.arena_)},
              list_        {meta::move(other.list_) } {}

    constexpr memory_pool& operator=(memory_pool&& other) noexcept = default;

    constexpr void* allocate_node() noexcept {
        if (list_.empty()) [[unlikely]]
            allocate_block();
        return list_.allocate();
    }

    constexpr void* try_allocate_node() noexcept {
        return list_.empty() ? nullptr : list_.allocate();
    }

    constexpr void* allocate_array(size_type count) noexcept {
        return allocate_array(count, node_size());
    }

    constexpr void* try_allocate_array(size_type count) noexcept {
        return try_allocate_array(count, node_size());
    }

    constexpr void deallocate_node(void* ptr) noexcept {
        list_.deallocate(ptr);
    }

    constexpr bool try_deallocate_node(void* ptr) noexcept {
        if (!arena_.contains(ptr)) [[unlikely]]
            return false;
        list_.deallocate(ptr);
        return true;
    }

    constexpr void deallocate_array(void* ptr, size_type count) noexcept {
        list_.deallocate(ptr, count * node_size());
    }

    constexpr bool try_deallocate_array(void* ptr, size_type count) noexcept {
        return try_deallocate_array(ptr, count, node_size());
    }

    // Returns the node size in the pool, this is either the same value as
    // in the constructor or `min_node_size` if the value was too small.
    constexpr size_type node_size() const noexcept {
        return list_.node_size();
    }

    // Returns the total amount of bytes remaining on the free list.
    // Divide it by `node_size()` to get the number of nodes that can be allocated.
    // Array allocations may lead to a growth even if the capacity_left left is big enough.
    constexpr size_type capacity() const noexcept {
        return list_.capacity() * node_size();
    }

    // Returns the size of the next memory block after the free list gets empty and the arena grows.
    constexpr size_type next_capacity() const noexcept {
        return list_.usable_size(arena_.next_block_size());
    }

    constexpr allocator_type& allocator() noexcept {
        return arena_.allocator();
    }

    static constexpr size_type min_node_size = memory_list::min_size;

    static constexpr size_type min_block_size(size_type node_size, size_type count) noexcept {
        return memory_block_stack::offset() + memory_list::min_block_size(node_size, count);
    }

private:
    constexpr auto info() const noexcept {
        return allocator_info{"salt::memory::memory_pool", this};
    }

    constexpr void allocate_block() noexcept {
        auto block = arena_.allocate_block();
        list_.insert(static_cast<std::byte*>(block.memory), block.size);
    }

    constexpr void* allocate_array(size_type count, size_type node_size) noexcept {
        auto* memory = list_.empty() ? nullptr : list_.allocate(count * node_size);
        if (!memory) {
            allocate_block();
            memory = list_.allocate(count * node_size);
            if (!memory) [[unlikely]]
                utility::terminate();
        }
        return memory;
    }

    constexpr void* try_allocate_array(size_type count, size_type node_size) noexcept {
        return list_.empty() ? nullptr : list_.allocate(count * node_size);
    }

    constexpr bool try_deallocate_array(void* ptr, size_type count, size_type node_size) noexcept {
        if (!arena_.contains(ptr))
            return false;
        list_.deallocate(ptr, count * node_size);
        return true;
    }

    memory_arena<allocator_type, IsCached> arena_;
    memory_list                            list_;

    friend allocator_traits<memory_pool>;
    friend composable_traits<memory_pool>;
};

template <typename PoolType, raw_allocator RawAllocator>
struct [[nodiscard]] allocator_traits<memory_pool<PoolType, RawAllocator>> final {
    using allocator_type  = memory_pool<PoolType, RawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using stateful        = meta::true_type;

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
        allocator.list_.deallocate(array, count * size);
        allocator.on_deallocate(count * size);
    }
    // clang-format on

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        return allocator.node_size();
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        return allocator.next_capacity();
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        return allocator.list_.alignment();
    }
};

template <typename PoolType, typename BlockOrRawAllocator>
struct [[nodiscard]] composable_traits<memory_pool<PoolType, BlockOrRawAllocator>> final {
    using allocator_type  = memory_pool<PoolType, BlockOrRawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    static constexpr void*
    try_allocate_node(allocator_type& allocator,
                      size_type       size     ,
                      size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (size      > allocator_traits::max_node_size(allocator) ||
            alignment > allocator_traits::max_alignment(allocator))
            return nullptr;
        return allocator.try_allocate_node();
    }

    static constexpr void*
    try_allocate_array(allocator_type& allocator,
                       size_type       count    ,
                       size_type       size     ,
                       size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (size         > allocator_traits::max_node_size (allocator) ||
            count * size > allocator_traits::max_array_size(allocator) ||
            alignment    > allocator_traits::max_alignment (allocator))
            return nullptr;
        return allocator.try_allocate_array(count, size);
    }

    static constexpr bool
    try_deallocate_node(allocator_type& allocator,
                        void*           node     ,
                        size_type       size     ,
                        size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (size      > allocator_traits::max_node_size(allocator) ||
            alignment > allocator_traits::max_alignment(allocator))
            return false;
        return allocator.try_deallocate_node(node);
    }

    static constexpr bool
    try_deallocate_array(allocator_type& allocator,
                         void*           array    ,
                         size_type       count    ,
                         size_type       size     ,
                         size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (size         > allocator_traits::max_node_size (allocator) ||
            count * size > allocator_traits::max_array_size(allocator) ||
            alignment    > allocator_traits::max_alignment (allocator))
            return false;
        return allocator.try_deallocate_array(array, count, size);
    }
    // clang-format on
};

} // namespace salt::memory