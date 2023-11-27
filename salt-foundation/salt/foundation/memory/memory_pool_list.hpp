#pragma once
#include <salt/foundation/memory/align.hpp>
#include <salt/foundation/memory/memory_arena.hpp>
#include <salt/foundation/memory/memory_pool_type.hpp>

#include <salt/foundation/memory/detail/fixed_stack.hpp>

namespace salt::memory {

// An stateful allocator that behaves as a collection of multiple `memory_pool` objects. It
// maintains a list of multiple free lists, whose types are controlled via the `PoolType` tags
// defined in `memory_pool_type.hpp`, each of a different size as defined in the `BucketType`
// (`identity_buckets` or `log2_buckets`). Allocating a node of given size will use the appropriate
// free list. This allocator is ideal for allocations in any order but with a predefined set of
// sizes, not only one size like `memory_pool`.
// clang-format off
template <
    typename PoolType            = node_pool,
    typename BucketType          = identity_buckets,
    typename BlockOrRawAllocator = default_allocator,
    bool     IsCached            = disable_caching
>
// clang-format on
class [[nodiscard]] memory_pool_list : default_leak_detector<detail::memory_pool_leak_handler> {
    using free_list       = typename PoolType::type;
    using free_list_array = detail::free_list_array<free_list, typename BucketType::type>;
    using fixed_stack     = detail::fixed_stack;
    using leak_detector   = default_leak_detector<detail::memory_pool_leak_handler>;

public:
    using allocator_type  = block_allocator_type<BlockOrRawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using pool_type       = PoolType;
    using bucket_type     = BucketType;
    using iterator        = typename free_list_array::iterator;
    using const_iterator  = typename free_list_array::const_iterator;

    // clang-format off
    template <typename... Args>
    constexpr memory_pool_list(size_type max_node_size, size_type block_size, Args&&... args) noexcept
            : arena_{block_size, meta::forward<Args>(args)...},
              stack_{allocate_block()                        },
              lists_{stack_, block_end(), max_node_size      } {}
    // clang-format on

    constexpr ~memory_pool_list() = default;

    constexpr memory_pool_list(memory_pool_list&& other) noexcept
            : leak_detector{meta::move(other)}, arena_{meta::move(other.arena_)},
              stack_{meta::move(other.stack_)}, lists_{meta::move(other.lists_)} {}

    constexpr memory_pool_list& operator=(memory_pool_list&& other) noexcept = default;

    constexpr void* allocate_node(size_type node_size) noexcept {
        auto& pool = lists_[node_size];
        if (pool.empty()) {
            auto block = reserve_memory(pool, block_size());
            pool.insert(block.memory, block.size);
        }

        auto* memory = pool.allocate();
        SALT_ASSERT(memory);
        return memory;
    }

    constexpr void* try_allocate_node(size_type node_size) noexcept {
        if (node_size > max_node_size())
            return nullptr;

        auto& pool = lists_[node_size];
        if (pool.empty()) {
            try_reserve_memory(pool, block_size());
            return pool.empty() ? nullptr : pool.allocate();
        }
        return pool.allocate();
    }

    constexpr void* allocate_array(size_type count, size_type node_size) noexcept {
        auto& pool   = lists_[node_size];
        auto* memory = pool.empty() ? nullptr : pool.allocate(count * node_size);
        if (!memory) {
            auto block = reserve_memory(pool, block_size());
            if (!block) {
                block = reserve_memory(pool, count * node_size);
            }

            pool.insert(block.memory, block.size);
            memory = pool.allocate(count * node_size);
            SALT_ASSERT(memory);
        }
        return memory;
    }

    constexpr void* try_allocate_array(size_type count, size_type node_size) noexcept {
        if (node_size > max_node_size())
            return nullptr;

        auto& pool = lists_[node_size];
        if (pool.empty()) {
            try_reserve_memory(pool, block_size());
            return pool.empty() ? nullptr : pool.allocate(count * node_size);
        }
        return pool.allocate(count * node_size);
    }

    constexpr void deallocate_node(void* ptr, size_type node_size) noexcept {
        lists_[node_size].deallocate(ptr);
    }

    constexpr bool try_deallocate_node(void* ptr, size_type node_size) noexcept {
        if (node_size > max_node_size() || !arena_.contains(ptr))
            return false;

        lists_[node_size].deallocate(ptr);
        return true;
    }

    constexpr void deallocate_array(void* ptr, size_type count, size_type node_size) noexcept {
        lists_[node_size].deallocate(ptr, count * node_size);
    }

    constexpr bool try_deallocate_array(void* ptr, size_type count, size_type node_size) noexcept {
        if (node_size > max_node_size() || !arena_.contains(ptr))
            return false;

        lists_[node_size].deallocate(ptr, count * node_size);
        return true;
    }

    constexpr void reserve(size_type node_size, size_type capacity) noexcept {
        SALT_ASSERT(node_size <= max_node_size());
        auto&                 pool  = lists_[node_size];
        [[maybe_unused]] auto block = reserve_memory(pool, capacity);
    }

    // Returns the maximum node size for which there is a free list.
    constexpr size_type max_node_size() const noexcept {
        return lists_.max_node_size();
    }

    // Returns the amount of nodes available in the free list for nodes of given size.
    // Array allocations may lead to a growth even if the capacity is big enough.
    constexpr size_type capacity(size_type node_size) const noexcept {
        SALT_ASSERT(node_size <= max_node_size());
        return lists_[node_size].capacity();
    }

    // Returns the amount of memory available in the arena not inside the free lists.
    // This is the size that can be inserted into the free lists without requesting more memory.
    // Array allocations may lead to a growth even if the capacity is big enough.
    constexpr size_type capacity() const noexcept {
        return size_type(block_end() - stack_.top());
    }

    // Returns the size of the next memory block after `capacity()` arena grows.
    constexpr size_type next_capacity() const noexcept {
        return arena_.next_block_size();
    }

    constexpr allocator_type& allocator() noexcept {
        return arena_.allocator();
    }

private:
    constexpr auto info() const noexcept {
        return allocator_info{"salt::memory::memory_pool_list", this};
    }

    constexpr auto block_size() const noexcept {
        return next_capacity() / lists_.size();
    }

    constexpr auto allocate_block() noexcept {
        return fixed_stack{arena_.allocate_block().memory};
    }

    constexpr auto block_end() const noexcept {
        auto block = arena_.current_block();
        return static_cast<const_iterator>(block.memory) + block.size;
    }

    constexpr bool fill(free_list& pool) noexcept {
        auto const top = stack_.top();
        if (auto const remaining = static_cast<size_type>(block_end() - top)) {
            if (auto const offset = align_offset(top, detail::max_alignment); offset < remaining) {
                stack_.advance(offset, debug_magic::alignment_memory);

                auto const size = remaining - offset;
                pool.insert(stack_.advance_return(size, debug_magic::internal_memory), size);
                return true;
            }
        }
        return false;
    }

    constexpr void try_reserve_memory(free_list& pool, size_type capacity) noexcept {
        auto* memory = stack_.allocate(block_end(), capacity, detail::max_alignment);
        if (!memory)
            fill(pool);
        else
            pool.insert(memory, capacity);
    }

    constexpr memory_block reserve_memory(free_list& pool, size_type capacity) noexcept {
        auto* memory = stack_.allocate(block_end(), capacity, detail::max_alignment);
        if (!memory) {
            fill(pool);
            stack_ = allocate_block();
            memory = stack_.allocate(block_end(), capacity, detail::max_alignment);

            if (!memory)
                return {};
        }
        return {memory, capacity};
    }

    memory_arena<allocator_type, IsCached> arena_;
    fixed_stack                            stack_;
    free_list_array                        lists_;

    friend allocator_traits<memory_pool_list>;
    friend composable_traits<memory_pool_list>;
};

// clang-format off
template <typename PoolType, typename BucketType, typename RawAllocator>
struct [[nodiscard]] allocator_traits<memory_pool_list<PoolType, BucketType, RawAllocator>> final {
    using allocator_type  = memory_pool_list<PoolType, BucketType, RawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using stateful        = meta::true_type;

    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment)
    {
        (void)alignment;
        auto* memory = allocator.allocate_node(size);
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
        allocator.deallocate_node(node, size);
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
        allocator.deallocate_array(array, count, size);
        allocator.on_deallocate(count * size);
    }

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        return allocator.max_node_size();
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        return allocator.next_capacity();
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        (void)allocator;
        return detail::max_alignment;
    }
};
// clang-format on

template <typename PoolType, typename BucketType, typename RawAllocator>
struct [[nodiscard]] composable_traits<memory_pool_list<PoolType, BucketType, RawAllocator>> final {
    using allocator_type  = memory_pool_list<PoolType, BucketType, RawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    static constexpr void*
    try_allocate_node(allocator_type& allocator,
                      size_type       size     ,
                      size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (alignment > allocator_traits::max_alignment(allocator))
            return nullptr;
        return allocator.try_allocate_node(size);
    }

    static constexpr void*
    try_allocate_array(allocator_type& allocator,
                       size_type       count    ,
                       size_type       size     ,
                       size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (count * size > allocator_traits::max_array_size(allocator) ||
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

        if (alignment > allocator_traits::max_alignment(allocator))
            return false;
        return allocator.try_deallocate_node(node, size);
    }

    static constexpr bool
    try_deallocate_array(allocator_type& allocator,
                         void*           array    ,
                         size_type       count    ,
                         size_type       size     ,
                         size_type       alignment) noexcept
    {
        using allocator_traits = allocator_traits<allocator_type>;

        if (count * size > allocator_traits::max_array_size(allocator) ||
            alignment    > allocator_traits::max_alignment (allocator))
            return false;
        return allocator.try_deallocate_array(array, count, size);
    }
    // clang-format on
};

} // namespace salt::memory