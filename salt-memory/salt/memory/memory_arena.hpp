#pragma once
#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>

#include <salt/memory/allocator_traits.hpp>
#include <salt/memory/default_allocator.hpp>
#include <salt/memory/detail/debug_helpers.hpp>
#include <salt/memory/memory_block.hpp>

namespace salt {

static constexpr inline bool enable_caching  = true;
static constexpr inline bool disable_caching = false;

namespace detail {

// Stores memory block in an intrusive linked list and allows LIFO access.
struct [[nodiscard]] Memory_block_stack final {
    using memory_block = salt::Memory_block;

    Memory_block_stack() noexcept = default;
    ~Memory_block_stack()         = default;

    constexpr Memory_block_stack(Memory_block_stack&& other) noexcept
            : head_{std::exchange(other.head_, nullptr)} {}

    constexpr Memory_block_stack& operator=(Memory_block_stack&& other) noexcept {
        head_ = std::exchange(other.head_, nullptr);
        return *this;
    }

    constexpr void push(memory_block block) noexcept {
        SALT_ASSERT(block.size >= sizeof(Node));
        SALT_ASSERT(is_aligned(block.memory, max_alignment));
        auto* next = ::new (block.memory) Node{head_, block.size - offset()};
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

    constexpr void steal_top(Memory_block_stack& other) noexcept {
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
        for (auto* node = head_; node; node = node->prev)
            ++count;
        return count;
    }

    constexpr bool contains(void const* ptr) const noexcept {
        auto* address = static_cast<std::byte const*>(ptr);
        for (auto* node = head_; node; node = node->prev) {
            auto* memory = static_cast<std::byte*>(static_cast<void*>(node));
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

template <bool Cached> struct [[nodiscard]] Memory_arena_cache;

template <> struct [[nodiscard]] Memory_arena_cache<enable_caching> {
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

    constexpr bool assign_block(Memory_block_stack& used) noexcept {
        if (cache_.empty()) [[unlikely]]
            return false;
        used.steal_top(cache_);
        return true;
    }

    template <typename BlockAllocator>
    constexpr void deallocate_block(BlockAllocator&, Memory_block_stack& used) noexcept {
        cache_.steal_top(used);
    }

    // clang-format off
    template <typename BlockAllocator>
    constexpr void shrink_to_fit(BlockAllocator& allocator) noexcept {
        Memory_block_stack to_deallocate;
        // Pop from cache and push to temporary stack
        while (!cache_.empty())
            to_deallocate.steal_top(cache_);
        // Now deallocate everything
        while (!to_deallocate.empty())
            allocator.deallocate_block(to_deallocate.pop());
    }
    // clang-format on

private:
    Memory_block_stack cache_;
};

template <> struct [[nodiscard]] Memory_arena_cache<disable_caching> {
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

    constexpr bool assign_block(Memory_block_stack&) noexcept {
        return false;
    }

    template <typename BlockAllocator>
    constexpr void deallocate_block(BlockAllocator& allocator, Memory_block_stack& used) noexcept {
        allocator.deallocate_block(used.pop());
    }

    // clang-format off
    template <typename BlockAllocator>
    constexpr void shrink_to_fit(BlockAllocator&) noexcept {}
    // clang-format on
};

} // namespace detail

// clang-format off
template <typename Allocator>
concept block_allocator =
    requires(Allocator allocator) {
        { allocator.block_size()     } -> std::same_as<std::size_t>;
        { allocator.allocate_block() } -> std::same_as<Memory_block>;
        { allocator.deallocate_block(std::declval<Memory_block>()) };
    };
// clang-format on
template <typename Allocator>
static constexpr inline bool is_block_allocator = block_allocator<Allocator>;

// A memory arena that manages huge memory blocks for a higher-level allocator. Some allocators
// like Memory_stack work on huge memory blocks, this class manages them for those allocators. It
// uses a BlockAllocator for the allocation of those blocks. The memory blocks in use are put onto
// a stack like structure, deallocation will pop from the top, so it is only possible to deallocate
// the last allocated block of the arena. By default, blocks are not really deallocated but stored
// in a cache.
template <block_allocator BlockAllocator, bool Cached = enable_caching>
class [[nodiscard]] Memory_arena : BlockAllocator, detail::Memory_arena_cache<Cached> {
    using memory_cache = detail::Memory_arena_cache<Cached>;
    using memory_block = Memory_block;
    using memory_stack = detail::Memory_block_stack;

public:
    using allocator_type  = BlockAllocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_cached       = std::bool_constant<Cached>;

    template <typename... Args>
    constexpr explicit Memory_arena(size_type block_size, Args&&... args)
            : allocator_type{block_size, std::forward<Args>(args)...} {
        SALT_ASSERT(block_size > min_block_size(0));
    }

    constexpr ~Memory_arena() {
        shrink_to_fit();
        while (!used_blocks_.empty())
            allocator_type::deallocate_block(used_blocks_.pop());
    }

    constexpr Memory_arena(Memory_arena&& other) noexcept
            : allocator_type{std::move(other)}, memory_cache{std::move(other)},
              used_blocks_{std::move(other.used_blocks_)} {}

    constexpr Memory_arena& operator=(Memory_arena&& other) noexcept = default;

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

// An allocator that uses a given RawAllocator for allocating the blocks. It calls the
// allocate_array() function with a node of size 1 and maximum alignment on the used allocator for
// the block allocation. The size of the next memory block will grow by a given factor after each
// allocation, allowing an amortized constant allocation time in the higher level allocator. The
// factor can be given as rational in the template parameter, default is 2.
// clang-format off
template <
    typename RawAllocator = Default_allocator,
    unsigned Numerator    = 2u,
    unsigned Denominator  = 1u
>
// clang-format on
class [[nodiscard]] Growing_block_allocator : allocator_traits<RawAllocator>::allocator_type {
    using memory_block     = Memory_block;
    using allocator_traits = allocator_traits<RawAllocator>;

    static_assert(float(Numerator) / Denominator >= 1.0f, "Invalid growth factor");

public:
    using allocator_type  = typename allocator_traits::allocator_type;
    using size_type       = typename allocator_traits::size_type;
    using difference_type = typename allocator_traits::difference_type;

    constexpr explicit Growing_block_allocator(size_type      block_size,
                                               allocator_type allocator = allocator_type{}) noexcept
            : allocator_type{std::move(allocator)}, block_size_{block_size} {}

    constexpr memory_block allocate_block() {
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

    static inline auto growth_factor() noexcept {
        static constexpr auto factor = float(Numerator) / Denominator;
        return factor;
    }

    static constexpr size_type new_block_size(size_type block_size) noexcept {
        return block_size * Numerator / Denominator;
    }

private:
    constexpr auto info() noexcept {
        return Allocator_info{"salt::Growing_block_allocator", this};
    }

    size_type block_size_;
};

// An allocator that allows only one block allocation. It can be used to prevent higher-level
// allocators from expanding. The one block allocation is performed through the allocate_array()
// function of the given RawAllocator.
template <typename RawAllocator = Default_allocator>
class [[nodiscard]] Fixed_block_allocator : allocator_traits<RawAllocator>::allocator_type {
    using memory_block     = Memory_block;
    using allocator_traits = allocator_traits<RawAllocator>;

public:
    using allocator_type  = typename allocator_traits::allocator_type;
    using size_type       = typename allocator_traits::size_type;
    using difference_type = typename allocator_traits::difference_type;

    constexpr explicit Fixed_block_allocator(size_type      block_size,
                                             allocator_type allocator = allocator_type{}) noexcept
            : allocator_type{std::move(allocator)}, block_size_{block_size} {}

    constexpr memory_block allocate_block() {
        if (block_size_) {
            auto         memory = allocator_traits::allocate_array(allocator(), block_size_, 1,
                                                                   detail::max_alignment);
            memory_block block(memory, block_size_);
            block_size_ = 0u;
            return block;
        }
        throw std::bad_alloc();
    }

    constexpr void deallocate_block(memory_block block) noexcept {
        // clang-format off
        detail::debug_check_pointer([&] { return block_size_ == 0u; }, info(), block.memory);
        allocator_traits::deallocate_array(
            allocator(), block.memory, block.size, 1, detail::max_alignment);
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
        return Allocator_info{"salt::Fixed_block_allocator", this};
    }

    size_type block_size_;
};

namespace detail {
template <typename RawAllocator>
using default_block_allocator = Growing_block_allocator<RawAllocator>;

template <template <typename...> typename Wrapper, typename BlockAllocator, typename... Args>
requires is_block_allocator<BlockAllocator>
constexpr auto make_block_allocator(std::size_t block_size, Args&&... args) {
    return BlockAllocator{block_size, std::forward<Args>(args)...};
}

template <template <typename...> typename Wrapper, typename RawAllocator>
constexpr auto make_block_allocator(std::size_t  block_size,
                                    RawAllocator allocator = RawAllocator()) {
    return Wrapper<RawAllocator>{block_size, std::move(allocator)};
}
} // namespace detail

template <typename BlockOrRawAllocator,
          template <typename...> typename BlockAllocator = detail::default_block_allocator>
using block_allocator_type =
        std::conditional_t<is_block_allocator<BlockOrRawAllocator>, BlockOrRawAllocator,
                           BlockAllocator<BlockOrRawAllocator>>;

// clang-format off
template <typename BlockOrRawAllocator, typename... Args>
constexpr block_allocator_type<BlockOrRawAllocator>
make_block_allocator(std::size_t block_size, Args&&... args) {
    return detail::make_block_allocator<detail::default_block_allocator, BlockOrRawAllocator>(
            block_size, std::forward<Args>(args)...);
}

template <template <typename...> typename BlockAllocator, typename BlockOrRawAllocator, typename... Args>
constexpr block_allocator_type<BlockOrRawAllocator, BlockAllocator>
make_block_allocator(std::size_t block_size, Args&&... args) {
    return detail::make_block_allocator<BlockAllocator, BlockOrRawAllocator>(
            block_size, std::forward<Args>(args)...);
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

} // namespace salt
