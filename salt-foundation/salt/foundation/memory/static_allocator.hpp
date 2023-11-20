#pragma once
#include <salt/config.hpp>

#include <salt/foundation/logging.hpp>
#include <salt/foundation/memory/detail/fixed_stack.hpp>
#include <salt/foundation/memory/memory_block.hpp>

namespace salt::memory {

// Storage for a `static_allocator`. Its constructor will take a reference to it and use it for its
// allocation. The storage type is simply a byte array aligned for maximum alignment.
template <std::size_t Size>
struct [[nodiscard]] static_allocator_storage final {
    alignas(max_alignment) std::byte storage[Size];
};

// A stateful `RawAllocator` that uses a fixed sized storage for the allocations.
// Deallocations are not supported, memory cannot be marked as freed.
struct [[nodiscard]] static_allocator {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using stateful        = meta::true_type;

    // clang-format off
    template <std::size_t Size> requires
        meta::same_size <static_allocator_storage<Size>, Size> and
        meta::same_align<static_allocator_storage<Size>, max_alignment>
    constexpr explicit static_allocator(static_allocator_storage<Size>& storage) noexcept
            : stack_{&storage}, end_{stack_.top() + Size} {}
    // clang-format on

    constexpr void* allocate_node(size_type size, size_type alignment) noexcept {
        auto memory = stack_.allocate(end_, size, alignment);
        SALT_ASSERT(memory, "salt::memory::static_allocator ran out of memory.");
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
    detail::fixed_stack stack_;
    std::byte const*    end_;
};

// An allocator that allocates the blocks from a fixed size storage. Deallocations are only allowed
// in reversed order which is guaranteed by Memory_arena.
struct [[nodiscard]] static_block_allocator {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    // clang-format off
    template <std::size_t Size> requires
        meta::same_size <static_allocator_storage<Size>, Size> and
        meta::same_align<static_allocator_storage<Size>, max_alignment>
    constexpr static_block_allocator(size_type                       block_size,
                                     static_allocator_storage<Size>& storage) noexcept
            : current_   {static_cast<std::byte*>(static_cast<void*>(&storage))},
              end_       {current_ + Size},
              block_size_{block_size}
    {
        SALT_ASSERT(block_size <= Size);
        SALT_ASSERT(Size % block_size == 0u);
    }

    constexpr static_block_allocator(static_block_allocator&& other) noexcept
            : current_   {utility::exchange(other.current_   , nullptr)},
              end_       {utility::exchange(other.end_       , nullptr)},
              block_size_{utility::exchange(other.block_size_, 0u     )} {}
    // clang-format on

    constexpr static_block_allocator& operator=(static_block_allocator&& other) noexcept {
        static_block_allocator tmp{meta::move(other)};
        current_    = tmp.current_;
        end_        = tmp.end_;
        block_size_ = tmp.block_size_;
        return *this;
    }

    constexpr memory_block allocate_block() noexcept {
        SALT_ASSERT(current_ + block_size_ <= end_,
                    "salt::memory::static_allocator ran out of memory.");
        auto memory = current_;
        current_ += block_size_;
        return {memory, block_size_};
    }

    constexpr void deallocate_block(memory_block block) noexcept {
        // clang-format off
        detail::debug_check_pointer([&] {
                    return current_ == static_cast<std::byte*>(block.memory) + block.size;
                }, allocator_info{"salt::memory::static_allocator", this}, block.memory);
        // clang-format on
        current_ -= block_size_;
    }

    constexpr size_type block_size() const noexcept {
        return block_size_;
    }

private:
    std::byte* current_;
    std::byte* end_;
    size_type  block_size_;
};

} // namespace salt::memory