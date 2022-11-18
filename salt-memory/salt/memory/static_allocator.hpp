#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/logger.hpp>
#include <salt/foundation/uninitialized_storage.hpp>
#include <salt/memory/detail/fixed_memory_stack.hpp>
#include <salt/memory/memory_block.hpp>

namespace salt {

// Storage for a Static_allocator. Its constructor will take a reference to it and use it for its
// allocation. The storage type is simply a byte array aligned for maximum alignment.
template <std::size_t Size>
using Static_allocator_storage = Uninitialized_storage<Size, detail::max_alignment>;

// A stateful allocator,RawAllocator that uses a fixed sized storage for the allocations.
// Deallocations are not supported, memory cannot be marked as freed.
struct [[nodiscard]] Static_allocator final {
    using is_stateful     = std::true_type;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    // clang-format off
    template <std::size_t Size> requires
        match_size     <Static_allocator_storage<Size>, Size> and
        match_alignment<Static_allocator_storage<Size>, detail::max_alignment>
    constexpr explicit Static_allocator(Static_allocator_storage<Size>& storage) noexcept
            : stack_{&storage}, end_{stack_.top() + Size} {}
    // clang-format on

    constexpr void* allocate_node(size_type size, size_type alignment) {
        auto memory = stack_.allocate(end_, size, alignment);
        if (!memory)
            throw std::bad_alloc();
        return memory;
    }

    constexpr void deallocate_node(void*, size_type, size_type) noexcept {}

    constexpr size_type max_node_size() const noexcept {
        return static_cast<size_type>(end_ - stack_.top());
    }

    constexpr size_type max_alignment() const noexcept {
        return static_cast<size_type>(-1);
    }

private:
    detail::Fixed_memory_stack stack_;
    std::byte const*           end_;
};

// An allocator that allocates the blocks from a fixed size storage. Deallocations are only allowed
// in reversed order which is guaranteed by Memory_arena.
struct [[nodiscard]] Static_block_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    // clang-format off
    template <std::size_t Size> requires
        match_size     <Static_allocator_storage<Size>, Size> and
        match_alignment<Static_allocator_storage<Size>, detail::max_alignment>
    constexpr Static_block_allocator(size_type                       block_size,
                                     Static_allocator_storage<Size>& storage) noexcept
            : current_{static_cast<std::byte*>(static_cast<void*>(&storage))},
              end_{current_ + Size}, block_size_{block_size} {
        SALT_ASSERT(block_size <= Size);
        SALT_ASSERT(Size % block_size == 0u);
    }

    constexpr Static_block_allocator(Static_block_allocator&& other) noexcept
            : current_   {std::exchange(other.current_   , nullptr)},
              end_       {std::exchange(other.end_       , nullptr)},
              block_size_{std::exchange(other.block_size_, 0      )} {}
    // clang-format on

    constexpr Static_block_allocator& operator=(Static_block_allocator&& other) noexcept {
        Static_block_allocator tmp{std::move(other)};
        current_    = tmp.current_;
        end_        = tmp.end_;
        block_size_ = tmp.block_size_;
        return *this;
    }

    constexpr Memory_block allocate_block() {
        if (current_ + block_size_ > end_)
            throw std::bad_alloc();
        auto memory = current_;
        current_ += block_size_;
        return {memory, block_size_};
    }

    constexpr void deallocate_block(Memory_block block) noexcept {
        // clang-format off
        detail::debug_check_pointer([&] {
                    return current_ == static_cast<std::byte*>(block.memory) + block.size;
                }, Allocator_info{"salt::Static_block_allocator", this}, block.memory);
        // clang-format on
        current_ -= block_size_;
    }

    constexpr size_type next_block_size() const noexcept {
        return block_size_;
    }

private:
    std::byte* current_;
    std::byte* end_;
    size_type  block_size_;
};

} // namespace salt
