#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/logger.hpp>
#include <salt/foundation/uninitialized_storage.hpp>
#include <salt/memory/detail/memory_stack.hpp>
#include <salt/memory/memory_block.hpp>

namespace salt {

template <std::size_t Size>
using static_allocator_storage = Uninitialized_storage<Size, detail::max_alignment>;

struct [[nodiscard]] Static_allocator final {
    using is_stateful = std::true_type;

    // clang-format off
    template <std::size_t Size>
    requires match_size     <static_allocator_storage<Size>, Size> and
             match_alignment<static_allocator_storage<Size>, detail::max_alignment>
    constexpr explicit Static_allocator(static_allocator_storage<Size>& storage) noexcept
            : stack_{&storage}, end_{stack_.top() + Size} {}
    // clang-format on

    constexpr void* allocate_node(std::size_t size, std::size_t alignment) {
        auto memory = stack_.allocate(end_, size, alignment);
        if (!memory)
            throw std::bad_alloc();
        return memory;
    }

    constexpr void deallocate_node(void*, std::size_t, std::size_t) noexcept {}

    constexpr std::size_t max_node_size() const noexcept {
        return static_cast<std::size_t>(end_ - stack_.top());
    }

    constexpr std::size_t max_alignment() const noexcept {
        return static_cast<std::size_t>(-1);
    }

private:
    detail::Fixed_memory_stack stack_;
    std::byte const*           end_;
};

struct [[nodiscard]] Static_block_allocator final {
    // clang-format off
    template <std::size_t Size>    
    requires match_size     <static_allocator_storage<Size>, Size> and
             match_alignment<static_allocator_storage<Size>, detail::max_alignment>
    constexpr Static_block_allocator(std::size_t                     block_size,
                                     static_allocator_storage<Size>& storage) noexcept
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

    constexpr std::size_t next_block_size() const noexcept {
        return block_size_;
    }

private:
    std::byte*  current_;
    std::byte*  end_;
    std::size_t block_size_;
};

} // namespace salt
