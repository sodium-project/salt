#pragma once
#include <salt/config.hpp>
#include <salt/foundation/logging.hpp>

#include <salt/foundation/memory/allocator_traits.hpp>
#include <salt/foundation/memory/construct_at.hpp>
#include <salt/foundation/memory/default_allocator.hpp>
#include <salt/foundation/memory/memory_block.hpp>

#include <salt/foundation/memory/detail/debug_helpers.hpp>

namespace salt::memory {

// clang-format off
inline constexpr bool  enable_caching = true;
inline constexpr bool disable_caching = false;
// clang-format on

namespace detail {

// Stores memory blocks in an intrusive linked list and allows LIFO access.
struct [[nodiscard]] memory_block_stack final {
    constexpr memory_block_stack() noexcept = default;
    constexpr ~memory_block_stack()         = default;

    constexpr memory_block_stack(memory_block_stack&& other) noexcept
            : head_{utility::exchange(other.head_, nullptr)} {}

    constexpr memory_block_stack& operator=(memory_block_stack&& other) noexcept {
        head_ = utility::exchange(other.head_, nullptr);
        return *this;
    }

    constexpr void push(memory_block block) noexcept {
        SALT_ASSERT(block.size >= sizeof(Node));
        SALT_ASSERT(is_aligned(block.memory, max_alignment));
        auto* next = construct_at(static_cast<Node*>(block.memory), head_, block.size - offset());
        head_      = next;
    }

    constexpr memory_block pop() noexcept {
        SALT_ASSERT(head_);
        auto* to_pop = head_;
        head_        = head_->prev;
        return {to_pop, to_pop->size + offset()};
    }

    constexpr memory_block top() const noexcept {
        SALT_ASSERT(head_);
        void* memory = static_cast<void*>(head_);
        return {static_cast<std::byte*>(memory) + offset(), head_->size};
    }

    constexpr void steal_top(memory_block_stack& other) noexcept {
        SALT_ASSERT(other.head_);
        auto* to_steal = other.head_;
        other.head_    = other.head_->prev;

        to_steal->prev = head_;
        head_          = to_steal;
    }

    constexpr bool empty() const noexcept {
        return nullptr == head_;
    }

    constexpr std::size_t size() const noexcept {
        std::size_t count = 0u;
        for (auto* node = head_; node; node = node->prev) {
            ++count;
        }
        return count;
    }

    constexpr bool contains(void const* ptr) const noexcept {
        auto* address = static_cast<std::byte const*>(ptr);
        for (auto* node = head_; node; node = node->prev) {
            auto* memory = static_cast<std::byte*>(static_cast<void*>(node)) + offset();
            if (address >= memory && address < memory + node->size)
                return true;
        }
        return false;
    }

    static constexpr std::size_t offset() noexcept {
        // Node size rounded up to the next multiple of max_alignment.
        return (sizeof(Node) / max_alignment + (sizeof(Node) % max_alignment != 0)) * max_alignment;
    }

private:
    struct [[nodiscard]] Node final {
        Node*       prev = nullptr;
        std::size_t size = 0u;
    };

    Node* head_ = nullptr;
};

// clang-format off
template <bool IsCached>
struct [[nodiscard]] memory_arena_cache;

template <>
struct [[nodiscard]] memory_arena_cache<enable_caching> {
protected:
    constexpr std::size_t size() const noexcept {
        return cache_.size();
    }

    constexpr std::size_t block_size() const noexcept {
        return cache_.top().size;
    }

    constexpr bool empty() const noexcept {
        return cache_.empty();
    }

    constexpr bool assign_block(memory_block_stack& used) noexcept {
        if (cache_.empty()) [[unlikely]]
            return false;
        used.steal_top(cache_);
        return true;
    }

    template <typename BlockAllocator>
    constexpr void deallocate_block(BlockAllocator&, memory_block_stack& used) noexcept {
        cache_.steal_top(used);
    }

    template <typename BlockAllocator>
    constexpr void shrink_to_fit(BlockAllocator& allocator) noexcept {
        memory_block_stack to_deallocate;
        // Pop from cache and push to temporary stack
        while (!cache_.empty())
            to_deallocate.steal_top(cache_);
        // Now deallocate everything
        while (!to_deallocate.empty())
            allocator.deallocate_block(to_deallocate.pop());
    }

private:
    memory_block_stack cache_;
};

template <>
struct [[nodiscard]] memory_arena_cache<disable_caching> {
protected:
    constexpr std::size_t size() const noexcept {
        return 0u;
    }

    constexpr std::size_t block_size() const noexcept {
        return 0u;
    }

    constexpr bool empty() const noexcept {
        return true;
    }

    constexpr bool assign_block(memory_block_stack&) noexcept {
        return false;
    }

    template <typename BlockAllocator>
    constexpr void deallocate_block(BlockAllocator& allocator, memory_block_stack& used) noexcept {
        allocator.deallocate_block(used.pop());
    }

    template <typename BlockAllocator>
    constexpr void shrink_to_fit(BlockAllocator&) noexcept {}
};
// clang-format on

} // namespace detail

// clang-format off
template <typename Allocator>
concept block_allocator =
    requires(Allocator allocator, memory_block block) {
        { allocator.block_size()            } noexcept -> meta::same_as<std::size_t>;
        { allocator.allocate_block  (/* */) } noexcept -> meta::same_as<memory_block>;
        { allocator.deallocate_block(block) } noexcept;
    };
template <typename Allocator>
inline constexpr bool is_block_allocator = block_allocator<Allocator>;
// clang-format on

// A memory arena that manages huge memory blocks for a higher-level allocator. Some allocators,
// like `memory_stack` work on huge memory blocks, this class manages them for those allocators.
// It uses a `BlockAllocator` for the allocation of those blocks. The memory blocks in use are
// put onto a stack, deallocation will pop from the top, so it is only possible to deallocate
// the last allocated block of the arena. By default, blocks are not really deallocated but
// stored in a `cache`. This can be disabled with the second template parameter.
template <block_allocator BlockAllocator, bool IsCached = enable_caching>
class [[nodiscard]] memory_arena : BlockAllocator, detail::memory_arena_cache<IsCached> {
    using memory_cache = detail::memory_arena_cache<IsCached>;
    using memory_stack = detail::memory_block_stack;

public:
    using allocator_type  = BlockAllocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_cached       = meta::bool_constant<IsCached>;

    template <typename... Args>
    constexpr explicit memory_arena(size_type block_size, Args&&... args)
            : allocator_type{block_size, meta::forward<Args>(args)...} {
        SALT_ASSERT(block_size > min_block_size(0));
    }

    constexpr ~memory_arena() {
        shrink_to_fit();
        while (!used_blocks_.empty())
            allocator_type::deallocate_block(used_blocks_.pop());
    }

    // clang-format off
    constexpr memory_arena(memory_arena&& other) noexcept
            : allocator_type{meta::move(other             )},
              memory_cache  {meta::move(other             )},
              used_blocks_  {meta::move(other.used_blocks_)} {}
    // clang-format on

    constexpr memory_arena& operator=(memory_arena&& other) noexcept = default;

    constexpr memory_block current_block() const noexcept {
        return used_blocks_.top();
    }

    constexpr memory_block allocate_block() {
        if (!memory_cache::assign_block(used_blocks_))
            used_blocks_.push(allocator_type::allocate_block());

        auto block = used_blocks_.top();
        detail::debug_fill_internal(block.memory, block.size, false);
        return block;
    }

    constexpr void deallocate_block() noexcept {
        auto block = used_blocks_.top();
        detail::debug_fill_internal(block.memory, block.size, true);
        memory_cache::deallocate_block(allocator(), used_blocks_);
    }

    constexpr bool contains(void const* ptr) const noexcept {
        return used_blocks_.contains(ptr);
    }

    constexpr void shrink_to_fit() noexcept {
        memory_cache::shrink_to_fit(allocator());
    }

    constexpr size_type size() const noexcept {
        return used_blocks_.size();
    }

    constexpr size_type cache_size() const noexcept {
        return memory_cache::size();
    }

    constexpr size_type capacity() const noexcept {
        return size() + cache_size();
    }

    constexpr size_type next_block_size() const noexcept {
        return memory_cache::empty() ? allocator_type::block_size() - memory_stack::offset()
                                     : memory_cache::block_size();
    }

    constexpr allocator_type& allocator() noexcept {
        return *this;
    }

    static constexpr size_type min_block_size(size_type byte_size) noexcept {
        return memory_stack::offset() + byte_size;
    }

private:
    memory_stack used_blocks_;
};

// An allocator that uses a given `RawAllocator` for allocating the blocks. It calls the
// `allocate_array` function with a node of size `1` and maximum alignment on the used allocator for
// the block allocation. The size of the next memory block will grow by a given factor after each
// allocation, allowing an amortized constant allocation time in the higher level allocator.
// The factor can be given as rational in the template parameter, default is `2`.
// clang-format off
template <
    typename RawAllocator = default_allocator,
    unsigned Numerator    = 2u,
    unsigned Denominator  = 1u
>
// clang-format on
class [[nodiscard]] growing_block_allocator : allocator_traits<RawAllocator>::allocator_type {
    using allocator_traits = allocator_traits<RawAllocator>;

    static_assert(float(Numerator) / Denominator >= 1.0f, "Invalid growth factor");

public:
    using allocator_type  = typename allocator_traits::allocator_type;
    using size_type       = typename allocator_traits::size_type;
    using difference_type = typename allocator_traits::difference_type;

    constexpr explicit growing_block_allocator(size_type      block_size,
                                               allocator_type allocator = allocator_type{}) noexcept
            : allocator_type{meta::move(allocator)}, block_size_{block_size} {}

    constexpr memory_block allocate_block() noexcept {
        auto*        memory = allocator_traits::allocate_array(allocator(), block_size_, 1,
                                                               detail::max_alignment);
        memory_block block{memory, block_size_};
        block_size_ = new_block_size(block_size_);
        return block;
    }

    constexpr void deallocate_block(memory_block block) noexcept {
        allocator_traits::deallocate_array(allocator(), block.memory, block.size, 1,
                                           detail::max_alignment);
    }

    constexpr size_type block_size() const noexcept {
        return block_size_;
    }

    constexpr allocator_type& allocator() noexcept {
        return *this;
    }

    static constexpr auto growth_factor() noexcept {
        static constexpr auto factor = float(Numerator) / Denominator;
        return factor;
    }

    static constexpr size_type new_block_size(size_type block_size) noexcept {
        return block_size * Numerator / Denominator;
    }

private:
    constexpr auto info() noexcept {
        return allocator_info{"salt::memory::growing_block_allocator", this};
    }

    size_type block_size_;
};

// An allocator that allows only one block allocation. It can be used to prevent higher-level
// allocators from expanding. The one block allocation is performed through the `allocate_array`
// function of the given `RawAllocator`.
template <typename RawAllocator = default_allocator>
class [[nodiscard]] fixed_block_allocator : allocator_traits<RawAllocator>::allocator_type {
    using allocator_traits = allocator_traits<RawAllocator>;

public:
    using allocator_type  = typename allocator_traits::allocator_type;
    using size_type       = typename allocator_traits::size_type;
    using difference_type = typename allocator_traits::difference_type;

    constexpr explicit fixed_block_allocator(size_type      block_size,
                                             allocator_type allocator = allocator_type{}) noexcept
            : allocator_type{meta::move(allocator)}, block_size_{block_size} {}

    constexpr memory_block allocate_block() noexcept {
        SALT_ASSERT(block_size_, "salt::memory::fixed_block_allocator ran out of memory.");
        auto         memory = allocator_traits::allocate_array(allocator(), block_size_, 1,
                                                               detail::max_alignment);
        memory_block block(memory, block_size_);
        block_size_ = 0u;
        return block;
    }

    constexpr void deallocate_block(memory_block block) noexcept {
        // clang-format off
        detail::debug_check_pointer([&] { return block_size_ == 0u; }, info(), block.memory);
        allocator_traits::deallocate_array(allocator(), block.memory, block.size, 1, detail::max_alignment);
        block_size_ = block.size;
        // clang-format on
    }

    constexpr size_type block_size() const noexcept {
        return block_size_;
    }

    constexpr allocator_type& allocator() noexcept {
        return *this;
    }

private:
    constexpr auto info() noexcept {
        return allocator_info{"salt::memory::fixed_block_allocator", this};
    }

    size_type block_size_;
};

template <typename RawAllocator>
using default_block_allocator = growing_block_allocator<RawAllocator>;

namespace detail {

template <template <typename...> typename Wrapper, typename Allocator, typename... Args>
    requires is_block_allocator<Allocator>
constexpr auto make_block_allocator_impl(std::size_t block_size, Args&&... args) {
    return Allocator{block_size, meta::forward<Args>(args)...};
}

template <template <typename...> typename Wrapper, typename RawAllocator>
    requires(not is_block_allocator<RawAllocator>)
constexpr auto make_block_allocator_impl(std::size_t  block_size,
                                         RawAllocator allocator = RawAllocator()) {
    return Wrapper<RawAllocator>{block_size, meta::move(allocator)};
}

} // namespace detail

template <typename BlockOrRawAllocator,
          template <typename...> typename BlockAllocator = default_block_allocator>
using block_allocator_type =
        meta::condition<is_block_allocator<BlockOrRawAllocator>, BlockOrRawAllocator,
                        BlockAllocator<BlockOrRawAllocator>>;

// clang-format off
template <typename BlockOrRawAllocator, typename... Args>
constexpr block_allocator_type<BlockOrRawAllocator>
make_block_allocator(std::size_t block_size, Args&&... args) {
    return detail::make_block_allocator_impl<default_block_allocator, BlockOrRawAllocator>(
            block_size, meta::forward<Args>(args)...);
}

template <template <typename...> typename BlockAllocator, typename BlockOrRawAllocator, typename... Args>
constexpr block_allocator_type<BlockOrRawAllocator, BlockAllocator>
make_block_allocator(std::size_t block_size, Args&&... args) {
    return detail::make_block_allocator_impl<BlockAllocator, BlockOrRawAllocator>(
            block_size, meta::forward<Args>(args)...);
}
// clang-format on

namespace literals {
constexpr std::size_t operator"" _KiB(unsigned long long value) noexcept {
    return std::size_t(value * 1024);
}

constexpr std::size_t operator"" _KB(unsigned long long value) noexcept {
    return std::size_t(value * 1000);
}

constexpr std::size_t operator"" _MiB(unsigned long long value) noexcept {
    return std::size_t(value * 1024 * 1024);
}

constexpr std::size_t operator"" _MB(unsigned long long value) noexcept {
    return std::size_t(value * 1000 * 1000);
}

constexpr std::size_t operator"" _GiB(unsigned long long value) noexcept {
    return std::size_t(value * 1024 * 1024 * 1024);
}

constexpr std::size_t operator"" _GB(unsigned long long value) noexcept {
    return std::size_t(value * 1000 * 1000 * 1000);
}
} // namespace literals

} // namespace salt::memory