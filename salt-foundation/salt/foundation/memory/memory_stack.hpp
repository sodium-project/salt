#pragma once
#define SALT_MEMORY_STACK_HAS_MIN_BLOCK_SIZE (1)

#include <salt/foundation/memory/detail/fixed_stack.hpp>
#include <salt/foundation/memory/memory_arena.hpp>

namespace salt::memory {

namespace detail {

struct [[nodiscard]] stack_marker final {
    std::size_t      index;
    std::byte*       top;
    std::byte const* end;

    // clang-format off
    constexpr stack_marker(std::size_t        idx,
                           fixed_stack const& stack,
                           std::byte const*   block_end) noexcept
            : index{idx}, top{stack.top()}, end{block_end} {}
    // clang-format on

    friend constexpr bool operator==(stack_marker const& lhs, stack_marker const& rhs) noexcept {
        if (lhs.index != rhs.index)
            return false;
        return lhs.top == rhs.top;
    }

    friend constexpr auto operator<=>(stack_marker const& lhs, stack_marker const& rhs) noexcept {
        if (lhs.index != rhs.index)
            return lhs.index <=> rhs.index;
        return lhs.top <=> rhs.top;
    }

    template <typename> friend struct memory_stack;
};

struct [[nodiscard]] memory_stack_leak_handler {
    constexpr void operator()(std::ptrdiff_t) noexcept {}
};

} // namespace detail

// clang-format off
// A stateful `RawAllocator` that provides stack-like (LIFO) allocations. It uses a `memory_arena`
// with a given `BlockOrRawAllocator` defaulting to `growing_block_allocator` to allocate huge
// blocks and saves a marker to the current top. Allocation simply moves this marker by the
// appropriate number of bytes and returns the pointer at the old marker position, deallocation
// is not directly supported, only setting the marker to a previously queried position.
template <typename BlockOrRawAllocator = default_allocator>
struct memory_stack : default_leak_detector<detail::memory_stack_leak_handler> {
    using allocator_type  = block_allocator_type<BlockOrRawAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using marker          = detail::stack_marker;

    template <typename... Args>
    constexpr explicit memory_stack(size_type size, Args&&... args)
            : arena_{size, meta::forward<Args>(args)...},
              stack_{arena_.allocate_block().memory    } {}

    constexpr void* allocate(size_type size, size_type alignment) noexcept {
        auto fence  = detail::debug_fence_size;
        auto offset = align_offset(stack_.top() + fence, alignment);

        if (auto const grow_size = fence + offset + size + fence;
            !stack_.top() || grow_size > capacity_left()) {
            auto block = arena_.allocate_block();
            stack_     = detail::fixed_stack(block.memory);
            offset     = align_offset(stack_.top() + fence, alignment);
        }
        return stack_.allocate_unchecked(size, offset);
    }

    constexpr void* try_allocate(size_type size, size_type alignment) noexcept {
        return stack_.allocate(end(), size, alignment);
    }

    constexpr marker top() const noexcept {
        return {arena_.size() - 1u, stack_, end()};
    }

    constexpr void unwind(marker stack_marker) noexcept {
        SALT_ASSERT(stack_marker <= top());

        auto const arena_end = arena_.size() - 1u;
        detail::debug_check_pointer(
                [&] { return stack_marker.index <= arena_end; }, info(), stack_marker.top);

        if (auto const to_deallocate = arena_end - stack_marker.index) {
            for (size_type i = 0; i < to_deallocate; ++i) {
                arena_.deallocate_block();
            }

            detail::debug_check_pointer(
                    [&] {
                        auto block = arena_.current_block();
                        return stack_marker.end ==
                               static_cast<std::byte*>(block.memory) + block.size;
                    }, info(), stack_marker.top);

            detail::debug_fill_free(stack_marker.top,
                                    size_type(stack_marker.end - stack_marker.top), 0);
            stack_ = detail::fixed_stack(stack_marker.top);
        } else {
            detail::debug_check_pointer(
                    [&] { return stack_.top() >= stack_marker.top; }, info(), stack_marker.top);
            stack_.unwind(stack_marker.top);
        }
    }

    constexpr void shrink_to_fit() noexcept {
        arena_.shrink_to_fit();
    }

    constexpr size_type capacity_left() const noexcept {
        return static_cast<size_type>(end() - stack_.top());
    }

    constexpr size_type next_capacity() const noexcept {
        return arena_.next_block_size();
    }

    constexpr allocator_type& allocator() noexcept {
        return arena_.allocator();
    }

    static constexpr size_type min_block_size(size_type size_bytes) noexcept {
        return detail::memory_block_stack::offset() + size_bytes;
    }

private:
    constexpr auto info() noexcept {
        return allocator_info{"salt::memory::memory_stack", this};
    }

    constexpr auto end() const noexcept {
        auto block = arena_.current_block();
        return static_cast<std::byte const*>(block.memory) + block.size;
    }

    memory_arena<allocator_type> arena_;
    detail::fixed_stack          stack_;

    friend allocator_traits<memory_stack>;
    friend composable_traits<memory_stack>;
};
// clang-format on

// Simple utility that automatically unwinds a `stack` to a previously saved location. A `stack` is
// anything that provides a `marker`, a `top()` function returning a `marker` and an `unwind()`
// function to unwind to a `marker`, like a `memory_stack`.
template <typename Stack = memory_stack<>>
struct [[nodiscard]] memory_stack_unwinder final {
    using stack_type  = Stack;
    using marker_type = typename stack_type::marker;

    constexpr memory_stack_unwinder(stack_type& stack, marker_type marker) noexcept
            : marker_{marker}, stack_{&stack} {}

    constexpr explicit memory_stack_unwinder(stack_type& stack) noexcept
            : memory_stack_unwinder(stack, stack.top()) {}

    constexpr ~memory_stack_unwinder() {
        if (stack_)
            stack_->unwind(marker_);
    }

    constexpr memory_stack_unwinder(memory_stack_unwinder&& other) noexcept
            : marker_{other.marker_}, stack_{other.stack_} {
        other.stack_ = nullptr;
    }

    constexpr memory_stack_unwinder& operator=(memory_stack_unwinder&& other) noexcept {
        if (stack_)
            stack_->unwind(marker_);

        marker_ = other.marker_;
        stack_  = utility::exchange(other.stack_, nullptr);
        return *this;
    }

    constexpr void release() noexcept {
        stack_ = nullptr;
    }

    constexpr void unwind() noexcept {
        SALT_ASSERT(will_unwind());
        stack_->unwind(marker_);
    }

    constexpr bool will_unwind() const noexcept {
        return stack_ != nullptr;
    }

    constexpr marker_type marker() const noexcept {
        SALT_ASSERT(will_unwind());
        return marker_;
    }

    constexpr stack_type& stack() const noexcept {
        SALT_ASSERT(will_unwind());
        return *stack_;
    }

private:
    marker_type marker_;
    stack_type* stack_;
};

// clang-format off
template <typename BlockAllocator>
struct [[nodiscard]] allocator_traits<memory_stack<BlockAllocator>> final {
    using allocator_type  = memory_stack<BlockAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;
    using is_stateful     = std::true_type;

    static constexpr void*
    allocate_node(allocator_type& allocator,
                  size_type       size     ,
                  size_type       alignment)
    {
        auto* memory = allocator.allocate(size, alignment);
        allocator.on_allocate(size);
        return memory;
    }

    static constexpr void*
    allocate_array(allocator_type& allocator,
                   size_type       count    ,
                   size_type       size     ,
                   size_type       alignment)
    {
        return allocate_node(allocator, count * size, alignment);
    }

    static constexpr void
    deallocate_node(allocator_type& allocator,
                    void*           node     ,
                    size_type       size     ,
                    size_type       alignment) noexcept
    {
        (void)node;
        (void)alignment;
        allocator.on_deallocate(size);
    }

    static constexpr void
    deallocate_array(allocator_type& allocator,
                     void*           array    ,
                     size_type       count    ,
                     size_type       size     ,
                     size_type       alignment) noexcept
    {
        deallocate_node(allocator, array, count * size, alignment);
    }

    static constexpr size_type max_node_size(allocator_type const& allocator) noexcept {
        return allocator.next_capacity();
    }

    static constexpr size_type max_array_size(allocator_type const& allocator) noexcept {
        return allocator.next_capacity();
    }

    static constexpr size_type max_alignment(allocator_type const& allocator) noexcept {
        (void)allocator;
        return size_type(-1);
    }
};

template <typename BlockAllocator>
struct [[nodiscard]] composable_traits<memory_stack<BlockAllocator>> final {
    using allocator_type  = memory_stack<BlockAllocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    static constexpr void*
    try_allocate_node(allocator_type& allocator,
                      size_type       size     ,
                      size_type       alignment) noexcept
    {
        return allocator.try_allocate(size, alignment);
    }

    static constexpr void*
    try_allocate_array(allocator_type& allocator,
                       size_type       count    ,
                       size_type       size     ,
                       size_type       alignment) noexcept
    {
        return allocator.try_allocate_array(count * size, alignment);
    }

    static constexpr bool
    try_deallocate_node(allocator_type& allocator,
                        void*           node     ,
                        size_type       size     ,
                        size_type       alignment) noexcept
    {
        (void)size;
        (void)alignment;
        return allocator.arena_.owns(node);
    }

    static constexpr bool
    try_deallocate_array(allocator_type& allocator,
                         void*           array    ,
                         size_type       count    ,
                         size_type       size     ,
                         size_type       alignment) noexcept
    {
        return try_deallocate_node(allocator, array, count, size, alignment);
    }
};
// clang-format on

} // namespace salt::memory