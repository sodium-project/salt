#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/uninitialized_storage.hpp>
#include <salt/memory/detail/memory_stack.hpp>

namespace salt {

template <std::size_t Size>
using static_allocator_storage = Uninitialized_storage<Size, detail::max_alignment>;

struct [[nodiscard]] Static_allocator final {
    using is_stateful = std::true_type;

    static_assert(sizeof(static_allocator_storage<1024>) == 1024);
    static_assert(alignof(static_allocator_storage<1024>) == detail::max_alignment);

    template <std::size_t Size>
    Static_allocator(static_allocator_storage<Size>& storage) noexcept
            : stack_{&storage}, end_{stack_.top() + Size} {}

    void* allocate_node(std::size_t size, std::size_t alignment);

    void deallocate_node(void*, std::size_t, std::size_t) noexcept {}

    std::size_t max_node_size() const noexcept {
        return static_cast<std::size_t>(end_ - stack_.top());
    }

    std::size_t max_alignment() const noexcept {
        return std::size_t(-1);
    }

private:
    Allocator_info info() const noexcept;

    detail::Fixed_memory_stack stack_;
    std::byte const*           end_;
};

} // namespace salt
