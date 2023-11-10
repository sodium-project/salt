#pragma once
#include <salt/foundation/memory/align.hpp>
#include <salt/foundation/memory/debugging.hpp>

#include <salt/foundation/memory/detail/debug_helpers.hpp>
#include <salt/foundation/memory/detail/utility.hpp>

#include <salt/foundation/memory/detail/free_list_helpers.hpp>

namespace salt::memory::detail {

// clang-format off
// Stores free blocks for a memory pool, memory blocks are fragmented and stored in a list.
struct [[nodiscard]] free_list final {
    using byte_type      = std::byte;
    using size_type      = std::size_t;
    using iterator       = byte_type*;
    using const_iterator = byte_type const*;

    static constexpr auto min_size      = sizeof(byte_type*);
    static constexpr auto min_alignment = alignof(byte_type*);

    static constexpr size_type min_block_size(size_type node_size, size_type node_count) noexcept {
        return (node_size < min_size ? min_size : node_size) * node_count;
    }

    constexpr explicit free_list(size_type node_size) noexcept
            : first_    {nullptr},
              node_size_{node_size > min_size ? node_size : min_size},
              capacity_ {0u} {}

    constexpr free_list(size_type node_size, void* memory, size_type size) noexcept
            : free_list{node_size} {
        insert(memory, size);
    }

    constexpr ~free_list() = default;

    constexpr free_list(free_list&& other) noexcept
            : first_    {detail::exchange(other.first_, nullptr)},
              node_size_{other.node_size_},
              capacity_ {detail::exchange(other.capacity_, 0u)} {}

    constexpr free_list& operator=(free_list&& other) noexcept {
        free_list tmp{meta::move(other)};
        first_     = tmp.first_;
        node_size_ = tmp.node_size_;
        capacity_  = tmp.capacity_;
        return *this;
    }

    constexpr void insert(void* memory, size_type size) noexcept {
        SALT_ASSERT(memory);
        SALT_ASSERT(is_aligned(memory, alignment()));
        debug_fill_internal(memory, size, false);
        insert_impl(memory, size);
    }

    constexpr void* allocate() noexcept {
        SALT_ASSERT(!empty());
        --capacity_;

        auto memory = first_;
        first_      = get_next(first_);
        return debug_fill_new(memory, node_size_, 0);
    }

    constexpr void* allocate(size_type n) noexcept {
        SALT_ASSERT(!empty());
        if (n <= node_size_)
            return allocate();

        auto range = find(first_, n, node_size_);
        if (!range.first) [[unlikely]]
            return nullptr;

        if (range.prev)
            set_next(range.prev, range.next);
        else
            first_ = range.next;
        capacity_ -= node_count(range, node_size_);

        return debug_fill_new(range.first, n, 0);
    }

    constexpr void deallocate(void* ptr) noexcept {
        ++capacity_;

        auto range = static_cast<iterator>(debug_fill_free(ptr, node_size_, 0));
        set_next(range, first_);
        first_ = range;
    }

    constexpr void deallocate(void* ptr, size_type n) noexcept {
        if (n <= node_size_)
            deallocate(ptr);
        else
            insert_impl(debug_fill_free(ptr, n, 0), n);
    }

    constexpr size_type alignment() const noexcept {
        return alignment_for(node_size_);
    }

    constexpr size_type node_size() const noexcept {
        return node_size_;
    }

    constexpr size_type usable_size(size_type size) const noexcept {
        return (size / node_size_) * node_size_;
    }

    constexpr size_type capacity() const noexcept {
        return capacity_;
    }

    constexpr bool empty() const noexcept {
        return nullptr == first_;
    }

private:
    constexpr void insert_impl(void* memory, size_type size) noexcept {
        auto node_count = size / node_size_;
        SALT_ASSERT(node_count > 0);

        auto range = static_cast<iterator>(memory);
        for (size_type i = 0u; i < node_count - 1; ++i) {
            set_next(range, range + node_size_);
            range += node_size_;
        }
        set_next(range, first_);
        first_     = static_cast<iterator>(memory);
        capacity_ += node_count;
    }

    iterator  first_;
    size_type node_size_;
    size_type capacity_;
};

// Stores free blocks for a memory pool, memory blocks are fragmented and stored in a list. Keeps
// the nodes ordered this allows array allocations, that is, consecutive nodes.
struct [[nodiscard]] ordered_free_list final {
    using byte_type      = std::byte;
    using size_type      = std::size_t;
    using iterator       = byte_type*;
    using const_iterator = byte_type const*;

    static constexpr auto min_size      = sizeof(byte_type*);
    static constexpr auto min_alignment = alignof(byte_type*);

    static constexpr size_type min_block_size(size_type node_size, size_type node_count) noexcept {
        return (node_size < min_size ? min_size : node_size) * node_count;
    }

    constexpr explicit ordered_free_list(size_type node_size) noexcept
            : node_size_{node_size > min_size ? node_size : min_size}, capacity_{0u},
              last_dealloc_{end_node()}, last_dealloc_prev_{begin_node()} {
        xor_set_next(begin_node(), nullptr, end_node());
        xor_set_next(end_node(), begin_node(), nullptr);
    }

    constexpr ordered_free_list(size_type node_size, void* memory, size_type size) noexcept
            : ordered_free_list{node_size} {
        insert(memory, size);
    }

    constexpr ~ordered_free_list() = default;

    constexpr ordered_free_list(ordered_free_list&& other) noexcept
            : node_size_{other.node_size_}, capacity_{other.capacity_} {
        if (!other.empty()) {
            auto* begin = xor_get_next(other.begin_node(), nullptr);
            auto* end   = xor_get_next(other.end_node(), nullptr);

            xor_set_next(begin_node(), nullptr, begin);
            xor_exchange(begin, other.begin_node(), begin_node());
            xor_exchange(end, other.end_node(), end_node());
            xor_set_next(end_node(), end, nullptr);

            other.capacity_ = 0u;
            xor_set_next(other.begin_node(), nullptr, other.end_node());
            xor_set_next(other.end_node(), other.begin_node(), nullptr);
        } else {
            xor_set_next(begin_node(), nullptr, end_node());
            xor_set_next(end_node(), begin_node(), nullptr);
        }

        last_dealloc_prev_ = begin_node();
        last_dealloc_      = xor_get_next(last_dealloc_prev_, nullptr);
    }

    constexpr ordered_free_list& operator=(ordered_free_list&& other) noexcept {
        ordered_free_list tmp{meta::move(other)};
        node_size_         = tmp.node_size_;
        capacity_          = tmp.capacity_;
        last_dealloc_      = tmp.last_dealloc_;
        last_dealloc_prev_ = tmp.last_dealloc_prev_;
        return *this;
    }

    constexpr void insert(void* memory, size_type size) noexcept {
        SALT_ASSERT(memory);
        SALT_ASSERT(is_aligned(memory, alignment()));
        debug_fill_internal(memory, size, false);
        insert_impl(memory, size);
    }

    constexpr void* allocate() noexcept {
        SALT_ASSERT(!empty());
        auto prev = begin_node();
        auto node = xor_get_next(prev, nullptr);
        auto next = xor_get_next(node, prev);

        xor_set_next(prev, nullptr, next);
        xor_exchange(next, node, prev);
        --capacity_;

        if (node == last_dealloc_) {
            last_dealloc_ = next;
            SALT_ASSERT(last_dealloc_prev_ == prev);
        } else if (node == last_dealloc_prev_) {
            last_dealloc_prev_ = prev;
            SALT_ASSERT(last_dealloc_ == next);
        }

        return debug_fill_new(node, node_size_, 0);
    }

    constexpr void* allocate(size_type size) noexcept {
        SALT_ASSERT(!empty());
        if (size <= node_size_)
            return allocate();

        auto range = xor_find<iterator>({begin_node(), end_node()}, size, node_size_);
        if (!range.first) [[unlikely]]
            return nullptr;

        xor_exchange(range.prev, range.first, range.next);
        xor_exchange(range.next, range.last, range.prev);
        capacity_ -= node_count(range, node_size_);

        if (less_equal(range.first, last_dealloc_) && less_equal(last_dealloc_, range.last)) {
            last_dealloc_      = range.next;
            last_dealloc_prev_ = range.prev;
        } else if (last_dealloc_prev_ == range.last) {
            SALT_ASSERT(last_dealloc_ == range.next);
            last_dealloc_prev_ = range.prev;
        }

        return debug_fill_new(range.first, size, 0);
    }

    constexpr void deallocate(void* memory) noexcept {
        auto node_next = static_cast<iterator>(debug_fill_free(memory, node_size_, 0));

        auto node =
                find_node(allocator_info{"salt::detail::ordered_free_list", this}, node_next,
                          {begin_node(), end_node()}, last_dealloc_, last_dealloc_prev_);
        // Links new node between prev and next
        xor_set_next(node_next, node.prev, node.next);
        xor_exchange(node.prev, node.next, node_next);
        xor_exchange(node.next, node.prev, node_next);
        ++capacity_;

        last_dealloc_prev_ = node.prev;
        last_dealloc_      = node_next;
    }

    constexpr void deallocate(void* memory, size_type size) noexcept {
        if (size <= node_size_) {
            deallocate(memory);
        } else {
            auto ptr  = debug_fill_free(memory, size, 0);
            auto prev = insert_impl(ptr, size);

            last_dealloc_prev_ = prev;
            last_dealloc_      = static_cast<iterator>(ptr);
        }
    }

    constexpr size_type alignment() const noexcept {
        return alignment_for(node_size_);
    }

    constexpr size_type node_size() const noexcept {
        return node_size_;
    }

    constexpr size_type usable_size(size_type size) const noexcept {
        return (size / node_size_) * node_size_;
    }

    constexpr size_type capacity() const noexcept {
        return capacity_;
    }

    constexpr bool empty() const noexcept {
        return 0u == capacity_;
    }

private:
    constexpr iterator insert_impl(void* memory, size_type size) noexcept {
        auto node_count = size / node_size_;
        SALT_ASSERT(node_count > 0);

        auto node = find_node(allocator_info{"salt::detail::ordered_free_list", this},
                              static_cast<iterator>(memory), {begin_node(), end_node()},
                              last_dealloc_, last_dealloc_prev_);
        xor_link_block(memory, node_size_, node_count, node);
        capacity_ += node_count;

        if (node.prev == last_dealloc_prev_) {
            last_dealloc_ = static_cast<iterator>(memory);
        }
        return node.prev;
    }

    constexpr iterator begin_node() noexcept {
        void* begin = &begin_proxy_;
        return static_cast<iterator>(begin);
    }

    constexpr iterator end_node() noexcept {
        void* end = &end_proxy_;
        return static_cast<iterator>(end);
    }

    size_type      node_size_, capacity_;
    iterator       last_dealloc_, last_dealloc_prev_;
    std::uintptr_t begin_proxy_, end_proxy_;
};
// clang-format on

#if SALT_MEMORY_DEBUG_DOUBLE_FREE
using  node_free_list = ordered_free_list;
using array_free_list = ordered_free_list;
#else
using  node_free_list = free_list;
using array_free_list = ordered_free_list;
#endif

} // namespace salt::memory::detail