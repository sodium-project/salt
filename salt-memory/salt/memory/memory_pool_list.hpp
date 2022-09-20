#pragma once
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/memory_list_array.hpp>
#include <salt/memory/detail/memory_stack.hpp>

#include <salt/memory/debugging.hpp>
#include <salt/memory/memory_arena.hpp>
#include <salt/memory/memory_pool_type.hpp>

namespace salt {

namespace detail {
struct Memory_pool_list_leak_handler {
    void operator()(std::ptrdiff_t amount) {
        get_leak_handler()({"salt::Memory_pool_list", this}, amount);
    }
};
} // namespace detail

// A BucketType for Memory_pool_list defining that there is a bucket, i.e. pool, for each
// size. That means that for each possible size up to an upper bound there will be a seperate free
// list. Allocating a node will not waste any memory.
struct [[nodiscard]] Identity_buckets final {
    using type = detail::Identity_access_policy;
};

// A BucketType for Memory_pool_list defining that there is a bucket, i.e. pool, for each
// power of two. That means for each power of two up to an upper bound there will be a separate
// free list. Allocating a node will only waste half of the memory.
struct [[nodiscard]] Log2_buckets final {
    using type = detail::Log2_access_policy;
};

// An stateful allocator that behaves as a collection of multiple Memory_pool objects. It maintains
// a list of multiple memory lists, whose types are controlled via the PoolType tags defined in
// memory_pool_type.hpp, each of a different size as defined in the BucketType (Identity_buckets or
// Log2_buckets). Allocating a node of given size will use the appropriate free list. This
// allocator is ideal for allocations in any order but with a predefined set of sizes, not only one
// size like Memory_pool.
// clang-format off
template <
    typename PoolType            = Node_pool,
    typename BucketType          = Identity_buckets,
    typename BlockOrRawAllocator = Default_allocator,
    bool     Cached              = disable_caching
>
// clang-format on
class [[nodiscard]] Memory_pool_list
        : detail::Default_leak_detector<detail::Memory_pool_list_leak_handler> {
    using memory_list       = typename PoolType::type;
    using memory_list_array = detail::Memory_list_array<memory_list, typename BucketType::type>;
    using memory_stack      = detail::Fixed_memory_stack;
    using leak_detector     = detail::Default_leak_detector<detail::Memory_pool_list_leak_handler>;

public:
    using allocator_type  = block_allocator_type<BlockOrRawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using pool_type       = PoolType;
    using bucket_type     = BucketType;
    using const_iterator  = typename memory_list_array::const_iterator;

    template <typename... Args>
    constexpr Memory_pool_list(size_type max_node_size, size_type block_size, Args&&... args)
            : arena_{block_size, std::forward<Args>(args)...}, stack_{allocate_block()},
              pools_{stack_, block_end(), max_node_size} {}

    constexpr ~Memory_pool_list() = default;

    constexpr Memory_pool_list(Memory_pool_list&& other) noexcept
            : leak_detector{std::move(other)}, arena_{std::move(other.arena_)},
              stack_{std::move(other.stack_)}, pools_{std::move(other.pools_)} {}

    constexpr Memory_pool_list& operator=(Memory_pool_list&& other) noexcept {
        leak_detector::operator=(std::move(other));
        arena_ = std::move(other.arena_);
        stack_ = std::move(other.stack_);
        pools_ = std::move(other.pools_);
        return *this;
    }

    constexpr void* allocate_node(size_type node_size) {
        auto& pool = pools_[node_size];
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

        auto& pool = pools_[node_size];
        if (pool.empty()) {
            try_reserve_memory(pool, next_capacity());
            return pool.empty() ? nullptr : pool.allocate();
        }
        return pool.allocate();
    }

    constexpr void* allocate_array(size_type count, size_type node_size) {
        auto& pool   = pools_[node_size];
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

        auto& pool = pools_[node_size];
        if (pool.empty()) {
            try_reserve_memory(pool, next_capacity());
            return pool.empty() ? nullptr : pool.allocate(count * node_size);
        }
        return pool.allocate(count * node_size);
    }

    constexpr void deallocate_node(void* node, size_type node_size) noexcept {
        pools_[node_size].deallocate(node);
    }

    constexpr bool try_deallocate_node(void* node, size_type node_size) noexcept {
        if (node_size > max_node_size() || !arena_.contains(node))
            return false;

        pools_[node_size].deallocate(node);
        return true;
    }

    constexpr void deallocate_array(void* ptr, size_type count, size_type node_size) noexcept {
        pools_[node_size].deallocate(ptr, count * node_size);
    }

    constexpr bool try_deallocate_array(void* ptr, size_type count, size_type node_size) noexcept {
        if (node_size > max_node_size() || !arena_.contains(ptr))
            return false;

        pools_[node_size].deallocate(ptr, count * node_size);
        return true;
    }

    constexpr void reserve(size_type node_size, size_type capacity) {
        SALT_ASSERT(node_size <= max_node_size());
        auto& pool = pools_[node_size];
        reserve_memory(pool, capacity);
    }

    constexpr size_type max_node_size() const noexcept {
        return pools_.max_node_size();
    }

    constexpr size_type size() const noexcept {
        return arena_.next_block_size();
    }

    constexpr size_type free_capacity(size_type node_size) const noexcept {
        SALT_ASSERT(node_size <= max_node_size());
        return pools_[node_size].capacity();
    }

    constexpr size_type capacity() const noexcept {
        return size_type(block_end() - stack_.top());
    }

    constexpr allocator_type& allocator() noexcept {
        return arena_.get_allocator();
    }

private:
    friend allocator_traits<Memory_pool_list>;

    constexpr auto info() const noexcept {
        return Allocator_info{"salt::Memory_pool_list", this};
    }

    constexpr size_type next_capacity() const noexcept {
        return arena_.next_block_size() / pools_.size();
    }

    constexpr memory_stack allocate_block() {
        return memory_stack{arena_.allocate_block().memory};
    }

    constexpr const_iterator block_end() const noexcept {
        auto block = arena_.current_block();
        return static_cast<const_iterator>(block.memory) + block.size;
    }

    constexpr bool fill(typename pool_type::type& pool) noexcept {
        if (auto remaining = size_type(block_end() - stack_.top())) {
            auto offset = detail::align_offset(stack_.top(), detail::max_alignment);
            if (offset < remaining) {
                detail::debug_fill(stack_.top(), offset, debug_magic::alignment_memory);
                pool.insert(stack_.top() + offset, remaining - offset);
                return true;
            }
        }
        return false;
    }

    constexpr void try_reserve_memory(typename pool_type::type& pool, size_type capacity) noexcept {
        auto* memory = stack_.allocate(block_end(), capacity, detail::max_alignment);
        if (!memory)
            fill(pool);
        else
            pool.insert(memory, capacity);
    }

    constexpr Memory_block reserve_memory(typename pool_type::type& pool, size_type capacity) {
        auto* memory = stack_.allocate(block_end(), capacity, detail::max_alignment);
        if (!memory) {
            fill(pool);
            stack_ = allocate_block();
            memory = stack_.allocate(block_end(), capacity, detail::max_alignment);
            SALT_ASSERT(memory);
        }
        return {memory, capacity};
    }

    Memory_arena<allocator_type, Cached> arena_;
    memory_stack                         stack_;
    memory_list_array                    pools_;
};

// clang-format off
template <typename PoolType, typename BucketType, typename RawAllocator, bool Cached>
struct [[nodiscard]] allocator_traits<
        Memory_pool_list<PoolType, BucketType, RawAllocator, Cached>> final {
    using allocator_type  = Memory_pool_list<PoolType, BucketType, RawAllocator, Cached>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     = std::true_type;

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

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        return allocator.max_node_size();
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        return allocator.size();
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        (void)allocator;
        return detail::max_alignment;
    }
};
// clang-format on

} // namespace salt