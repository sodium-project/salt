#pragma once
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/memory_ranges.hpp>

namespace salt::detail {

// Stores free blocks for a memory pool, memory blocks are fragmented and stored in a list.
struct [[nodiscard]] Unordered_memory_list final {
    using byte_type      = std::byte;
    using size_type      = std::size_t;
    using iterator       = byte_type*;
    using const_iterator = byte_type const*;
    using node_range     = Random_access_range<iterator>;
    using memory_range   = Contiguous_range<iterator>;

    explicit Unordered_memory_list(size_type node_size) noexcept;

    Unordered_memory_list(size_type node_size, void* memory, size_type size) noexcept;

    ~Unordered_memory_list() = default;

    Unordered_memory_list(Unordered_memory_list&& other) noexcept;

    Unordered_memory_list& operator=(Unordered_memory_list&& other) noexcept;

    void insert(void* memory, size_type size) noexcept;

    void* allocate() noexcept;

    void* allocate(size_type n) noexcept;

    void deallocate(void* ptr) noexcept;

    void deallocate(void* ptr, size_type n) noexcept;

    size_type alignment() const noexcept {
        return alignment_for(node_size_);
    }

    size_type node_size() const noexcept {
        return node_size_;
    }

    size_type usable_size(size_type size) const noexcept {
        return (size / node_size_) * node_size_;
    }

    size_type capacity() const noexcept {
        return capacity_;
    }

    bool empty() const noexcept {
        return nullptr == first_;
    }

    static constexpr auto min_size      = sizeof(byte_type*);
    static constexpr auto min_alignment = alignof(byte_type*);

    static constexpr size_type min_block_size(size_type node_size, size_type node_count) noexcept {
        return (node_size < min_size ? min_size : node_size) * node_count;
    }

private:
    void insert_impl(void* memory, size_type size) noexcept;

    iterator  first_;
    size_type node_size_;
    size_type capacity_;
};

// Stores free blocks for a memory pool, memory blocks are fragmented and stored in a list. Keeps
// the nodes ordered this allows array allocations, that is, consecutive nodes.
struct [[nodiscard]] Memory_list final {
    using byte_type      = std::byte;
    using size_type      = std::size_t;
    using iterator       = byte_type*;
    using const_iterator = byte_type const*;
    using node_range     = Random_access_range<iterator>;
    using memory_range   = Contiguous_range<iterator>;

    explicit Memory_list(size_type node_size) noexcept;

    Memory_list(size_type node_size, void* memory, size_type size) noexcept;

    ~Memory_list() = default;

    Memory_list(Memory_list&& other) noexcept;

    Memory_list& operator=(Memory_list&& other) noexcept;

    void insert(void* memory, size_type size) noexcept;

    void* allocate() noexcept;

    void* allocate(size_type size) noexcept;

    void deallocate(void* memory) noexcept;

    void deallocate(void* memory, size_type size) noexcept;

    size_type alignment() const noexcept {
        return alignment_for(node_size_);
    }

    size_type node_size() const noexcept {
        return node_size_;
    }

    size_type usable_size(size_type size) const noexcept {
        return (size / node_size_) * node_size_;
    }

    size_type capacity() const noexcept {
        return capacity_;
    }

    bool empty() const noexcept {
        return 0u == capacity_;
    }

    static constexpr auto min_size      = sizeof(byte_type*);
    static constexpr auto min_alignment = alignof(byte_type*);

    static constexpr size_type min_block_size(size_type node_size, size_type node_count) noexcept {
        return (node_size < min_size ? min_size : node_size) * node_count;
    }

private:
    iterator insert_impl(void* memory, size_type size) noexcept;

    iterator begin_node() noexcept;
    iterator end_node() noexcept;

    size_type node_size_;
    size_type capacity_;

    node_range  node_;
    Proxy_range proxy_;
};

#if SALT_MEMORY_DEBUG_DOUBLE_FREE
using Node_memory_list  = Memory_list;
using Array_memory_list = Memory_list;
#else
using Node_memory_list  = Unordered_memory_list;
using Array_memory_list = Memory_list;
#endif

} // namespace salt::detail