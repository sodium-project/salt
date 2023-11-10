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
            auto block = reserve_memory(pool, next_capacity());
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
            try_reserve_memory(pool, next_capacity());
            return pool.empty() ? nullptr : pool.allocate();
        }
        return pool.allocate();
    }

    constexpr void* allocate_array(size_type count, size_type node_size) noexcept {
        auto& pool   = lists_[node_size];
        auto* memory = pool.empty() ? nullptr : pool.allocate(count * node_size);
        if (!memory) {
            auto block = reserve_memory(pool, next_capacity());
            pool.insert(block.memory, block.size);

            memory = pool.allocate(count * node_size);
            if (!memory) {
                block = reserve_memory(pool, count * node_size);
                pool.insert(block.memory, block.size);

                memory = pool.allocate(count * node_size);
                SALT_ASSERT(memory);
            }
        }
        return memory;
    }

    constexpr void* try_allocate_array(size_type count, size_type node_size) noexcept {
        if (node_size > max_node_size())
            return nullptr;

        auto& pool = lists_[node_size];
        if (pool.empty()) {
            try_reserve_memory(pool, next_capacity());
            return pool.empty() ? nullptr : pool.allocate(count * node_size);
        }
        return pool.allocate(count * node_size);
    }

    constexpr void deallocate_node(void* node, size_type node_size) noexcept {
        lists_[node_size].deallocate(node);
    }

    constexpr bool try_deallocate_node(void* node, size_type node_size) noexcept {
        if (node_size > max_node_size() || !arena_.contains(node))
            return false;

        lists_[node_size].deallocate(node);
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
        auto& pool = lists_[node_size];
        reserve_memory(pool, capacity);
    }

    constexpr size_type max_node_size() const noexcept {
        return lists_.max_node_size();
    }

    constexpr size_type size() const noexcept {
        return arena_.next_block_size();
    }

    constexpr size_type capacity(size_type node_size) const noexcept {
        SALT_ASSERT(node_size <= max_node_size());
        return lists_[node_size].capacity();
    }

    constexpr size_type capacity() const noexcept {
        return size_type(block_end() - stack_.top());
    }

    constexpr allocator_type& allocator() noexcept {
        return arena_.get_allocator();
    }

private:
    constexpr auto info() const noexcept {
        return allocator_info{"salt::memory::memory_pool_list", this};
    }

    constexpr auto next_capacity() const noexcept {
        return arena_.next_block_size() / lists_.size();
    }

    constexpr auto allocate_block() noexcept {
        return fixed_stack{arena_.allocate_block().memory};
    }

    constexpr const_iterator block_end() const noexcept {
        auto block = arena_.current_block();
        return static_cast<const_iterator>(block.memory) + block.size;
    }

    constexpr bool fill(typename pool_type::type& pool) noexcept {
        if (auto remaining = size_type(block_end() - stack_.top())) {
            auto offset = align_offset(stack_.top(), max_alignment);
            if (offset < remaining) {
                detail::debug_fill(stack_.top(), offset, debug_magic::alignment_memory);
                pool.insert(stack_.top() + offset, remaining - offset);
                return true;
            }
        }
        return false;
    }

    constexpr void try_reserve_memory(typename pool_type::type& pool, size_type capacity) noexcept {
        auto* memory = stack_.allocate(block_end(), capacity, max_alignment);
        if (!memory)
            fill(pool);
        else
            pool.insert(memory, capacity);
    }

    constexpr memory_block reserve_memory(typename pool_type::type& pool,
                                          size_type                 capacity) noexcept {
        auto* memory = stack_.allocate(block_end(), capacity, max_alignment);
        if (!memory) {
            fill(pool);
            stack_ = allocate_block();
            memory = stack_.allocate(block_end(), capacity, max_alignment);
            SALT_ASSERT(memory);
        }
        return {memory, capacity};
    }

    memory_arena<allocator_type, IsCached> arena_;
    fixed_stack                            stack_;
    free_list_array                        lists_;

    friend allocator_traits<memory_pool_list>;
    friend composable_traits<memory_pool_list>;
};

} // namespace salt::memory