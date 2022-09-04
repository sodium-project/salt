#pragma once
#include <salt/config.hpp>
#include <salt/foundation/logger.hpp>

#include <salt/memory/allocator_traits.hpp>
#include <salt/memory/default_allocator.hpp>
#include <salt/memory/detail/debug_helpers.hpp>
#include <salt/memory/memory_block.hpp>

namespace salt {

constexpr bool arena_enable_caching  = true;
constexpr bool arena_disable_caching = false;

namespace detail {

// stores memory block in an intrusive linked list and allows LIFO access
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
        auto* current_node = head_;
        head_              = head_->prev;
        return {current_node, current_node->size + offset()};
    }

    constexpr void steal_top(Memory_block_stack& other) noexcept {
        SALT_ASSERT(other.head_);
        auto* other_head = other.head_;
        other.head_      = other.head_->prev;

        other_head->prev = head_;
        head_            = other_head;
    }

    constexpr memory_block top() const noexcept {
        SALT_ASSERT(head_);
        void* memory = static_cast<void*>(head_);
        return {static_cast<std::byte*>(memory) + offset(), head_->size};
    }

    constexpr bool empty() const noexcept {
        return head_ == nullptr;
    }

    constexpr bool owns(void const* ptr) const noexcept {
        auto* address = static_cast<std::byte const*>(ptr);
        for (auto* current_node = head_; current_node; current_node = current_node->prev) {
            auto* memory = static_cast<std::byte*>(static_cast<void*>(current_node));
            if (address >= memory && address < memory + current_node->size)
                return true;
        }
        return false;
    }

    constexpr std::size_t size() const noexcept {
        std::size_t count = 0u;
        for (auto* current_node = head_; current_node; current_node = current_node->prev)
            ++count;
        return count;
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

template <bool Cached> struct Memory_arena_cache;

template <> struct [[nodiscard]] Memory_arena_cache<arena_enable_caching> {
protected:
    constexpr bool empty() const noexcept {
        return cache_.empty();
    }

    constexpr std::size_t size() const noexcept {
        return cache_.size();
    }

    constexpr std::size_t block_size() const noexcept {
        return cache_.top().size;
    }

    constexpr bool assign_block(Memory_block_stack& used) noexcept {
        if (cache_.empty()) [[unlikely]]
            return false;
        used.steal_top(cache_);
        return true;
    }

    template <typename BlockAllocator>
    void deallocate_block(BlockAllocator&, Memory_block_stack& used) noexcept {
        cache_.steal_top(used);
    }

    template <typename BlockAllocator> void clear(BlockAllocator& allocator) noexcept {
        Memory_block_stack to_deallocate;
        // Pop from cache and push to temporary stack
        while (!cache_.empty())
            to_deallocate.steal_top(cache_);
        // Now deallocate everything
        while (!to_deallocate.empty())
            allocator.deallocate_block(to_deallocate.pop());
    }

private:
    Memory_block_stack cache_;
};

template <> struct [[nodiscard]] Memory_arena_cache<arena_disable_caching> {
protected:
    constexpr bool empty() const noexcept {
        return true;
    }

    constexpr std::size_t size() const noexcept {
        return 0u;
    }

    constexpr std::size_t block_size() const noexcept {
        return 0u;
    }

    constexpr bool assign_block(Memory_block_stack&) noexcept {
        return false;
    }

    template <typename BlockAllocator>
    void deallocate_block(BlockAllocator& allocator, Memory_block_stack& used) noexcept {
        allocator.deallocate_block(used.pop());
    }

    template <typename BlockAllocator> void clear(BlockAllocator&) noexcept {}
};

} // namespace detail

template <typename BlockAllocator, bool Cached = arena_enable_caching>
class [[nodiscard]] Memory_arena : BlockAllocator, detail::Memory_arena_cache<Cached> {
    using cache              = detail::Memory_arena_cache<Cached>;
    using memory_block       = Memory_block;
    using memory_block_stack = detail::Memory_block_stack;

public:
    using allocator_type  = BlockAllocator;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_cached       = std::integral_constant<bool, Cached>;

    template <typename... Args>
    explicit Memory_arena(size_type block_size, Args&&... args)
            : allocator_type{block_size, std::forward<Args>(args)...} {
        SALT_ASSERT(block_size > min_block_size(0));
    }

    ~Memory_arena() noexcept {
        clear();
        while (!used_.empty())
            allocator_type::deallocate_block(used_.pop());
    }

    Memory_arena(Memory_arena&& other) noexcept
            : allocator_type(std::move(other)), cache(std::move(other)),
              used_(std::move(other.used_)) {}

    Memory_arena& operator=(Memory_arena&& other) noexcept = default;

    memory_block current_block() const noexcept {
        return used_.top();
    }

    memory_block allocate_block() {
        if (!cache::assign_block(used_))
            used_.push(allocator_type::allocate_block());

        auto block = used_.top();
        detail::debug_fill_internal(block.memory, block.size, false);
        return block;
    }

    void deallocate_block() noexcept {
        auto block = used_.top();
        detail::debug_fill_internal(block.memory, block.size, true);
        cache::deallocate_block(allocator(), used_);
    }

    bool owns(void const* ptr) const noexcept {
        return used_.owns(ptr);
    }

    void clear() noexcept {
        cache::clear(allocator());
    }

    size_type size() const noexcept {
        return used_.size();
    }

    size_type cache_size() const noexcept {
        return cache::size();
    }

    size_type capacity() const noexcept {
        return size() + cache_size();
    }

    size_type next_block_size() const noexcept {
        return cache::empty() ? allocator_type::next_block_size() - memory_block_stack::offset()
                              : cache::block_size();
    }

    allocator_type& allocator() noexcept {
        return *this;
    }

    static constexpr size_type min_block_size(size_type byte_size) noexcept {
        return memory_block_stack::offset() + byte_size;
    }

private:
    memory_block_stack used_;
};

template <typename RawAllocator = Default_allocator, unsigned Numerator = 2,
          unsigned Denominator = 1>
class [[nodiscard]] Growing_block_allocator : allocator_traits<RawAllocator>::allocator_type {
    static_assert(float(Numerator) / Denominator >= 1.0, "Invalid growth factor");

public:
    using memory_block    = Memory_block;
    using allocator_type  = typename allocator_traits<RawAllocator>::allocator_type;
    using size_type       = typename allocator_traits<RawAllocator>::size_type;
    using difference_type = typename allocator_traits<RawAllocator>::difference_type;

    explicit Growing_block_allocator(size_type      block_size,
                                     allocator_type allocator = allocator_type{}) noexcept
            : allocator_type{std::move(alloc)}, block_size_{block_size} {}

    memory_block allocate_block() {
        auto* memory = allocator_traits<RawAllocator>::allocate_array(allocator(), block_size_, 1,
                                                                      detail::max_alignment);
        memory_block block(memory, block_size_);
        block_size_ = grow_block_size(block_size_);
        return block;
    }

    void deallocate_block(memory_block block) noexcept {
        allocator_traits<RawAllocator>::deallocate_array(allocator(), block.memory, block.size, 1,
                                                         detail::max_alignment);
    }

    size_type next_block_size() const noexcept {
        return block_size_;
    }

    allocator_type& allocator() noexcept {
        return *this;
    }

    static float growth_factor() noexcept {
        static constexpr auto factor = float(Numerator) / Denominator;
        return factor;
    }

    static size_type grow_block_size(size_type block_size) noexcept {
        return block_size * Numerator / Denominator;
    }

private:
    auto info() noexcept {
        return Allocator_info{"salt::Growing_block_allocator", this};
    }

    size_type block_size_;
};

template <typename RawAllocator = Default_allocator>
class [[nodiscard]] Fixed_block_allocator : allocator_traits<RawAllocator>::allocator_type {
public:
    using memory_block    = Memory_block;
    using allocator_type  = typename allocator_traits<RawAllocator>::allocator_type;
    using size_type       = typename allocator_traits<RawAllocator>::size_type;
    using difference_type = typename allocator_traits<RawAllocator>::difference_type;

    explicit Fixed_block_allocator(size_type      block_size,
                                   allocator_type allocator = allocator_type{}) noexcept
            : allocator_type{std::move(allocator)}, block_size_{block_size} {}

    memory_block allocate_block() {
        if (block_size_) {
            auto mem = allocator_traits<RawAllocator>::allocate_array(allocator(), block_size_, 1,
                                                                      detail::max_alignment);
            memory_block block(mem, block_size_);
            block_size_ = 0u;
            return block;
        }
        throw std::bad_alloc();
    }

    void deallocate_block(memory_block block) noexcept {
        // clang-format off
        detail::debug_check_pointer([&] { return block_size_ == 0u; }, info(), block.memory);
        allocator_traits<RawAllocator>::deallocate_array(allocator(), block.memory, block.size, 1,
                                                         detail::max_alignment);
        block_size_ = block.size;
        // clang-format on
    }

    size_type next_block_size() const noexcept {
        return block_size_;
    }

    allocator_type& allocator() noexcept {
        return *this;
    }

private:
    auto info() noexcept {
        return Allocator_info{"salt::Fixed_block_allocator", this};
    }

    size_type block_size_;
};

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
