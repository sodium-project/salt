#pragma once
#include <salt/memory/detail/align.hpp>

namespace salt::detail {

struct [[nodiscard]] Unordered_free_list final {
    static constexpr auto min_element_size      = sizeof(std::byte*);
    static constexpr auto min_element_alignment = alignof(std::byte*);

    static constexpr std::size_t min_block_size(std::size_t node_size,
                                                std::size_t number_of_nodes) noexcept {
        return (node_size < min_element_size ? min_element_size : node_size) * number_of_nodes;
    }

    explicit Unordered_free_list(std::size_t node_size) noexcept;

    Unordered_free_list(std::size_t node_size, void* memory, std::size_t size) noexcept;

    ~Unordered_free_list() = default;

    Unordered_free_list(Unordered_free_list&& other) noexcept;

    Unordered_free_list& operator=(Unordered_free_list&& other) noexcept;

    void insert(void* memory, std::size_t size) noexcept;

    void* allocate() noexcept;

    void* allocate(std::size_t n) noexcept;

    void deallocate(void* ptr) noexcept;

    void deallocate(void* ptr, std::size_t n) noexcept;

    std::size_t alignment() const noexcept {
        return alignment_for(node_size_);
    }

    std::size_t node_size() const noexcept {
        return node_size_;
    }

    std::size_t usable_size(std::size_t size) const noexcept {
        return (size / node_size_) * node_size_;
    }

    std::size_t capacity() const noexcept {
        return capacity_;
    }

    bool empty() const noexcept {
        return nullptr == first_;
    }

private:
    void insert_impl(void* memory, std::size_t size) noexcept;

    std::byte*  first_;
    std::size_t node_size_;
    std::size_t capacity_;
};

struct [[nodiscard]] Free_list final {
    static constexpr auto min_element_size      = sizeof(std::byte*);
    static constexpr auto min_element_alignment = alignof(std::byte*);

    static constexpr std::size_t min_block_size(std::size_t node_size,
                                                std::size_t number_of_nodes) noexcept {
        return (node_size < min_element_size ? min_element_size : node_size) * number_of_nodes;
    }

    explicit Free_list(std::size_t node_size) noexcept;

    Free_list(std::size_t node_size, void* memory, std::size_t size) noexcept;

    ~Free_list() = default;

    Free_list(Free_list&& other) noexcept;

    Free_list& operator=(Free_list&& other) noexcept;

    void insert(void* memory, std::size_t size) noexcept;

    void* allocate() noexcept;

    void* allocate(std::size_t n) noexcept;

    void deallocate(void* ptr) noexcept;

    void deallocate(void* ptr, std::size_t n) noexcept;

    std::size_t alignment() const noexcept {
        return alignment_for(node_size_);
    }

    std::size_t node_size() const noexcept {
        return node_size_;
    }

    std::size_t usable_size(std::size_t size) const noexcept {
        return (size / node_size_) * node_size_;
    }

    std::size_t capacity() const noexcept {
        return capacity_;
    }

    bool empty() const noexcept {
        return 0u == capacity_;
    }

private:
    std::byte* insert_impl(void* memory, std::size_t size) noexcept;

    std::byte* begin() noexcept;
    std::byte* end() noexcept;

    std::uintptr_t begin_proxy_;
    std::uintptr_t end_proxy_;
    std::size_t    node_size_;
    std::size_t    capacity_;

    std::byte* prev_dealloc_;
    std::byte* last_dealloc_;
};

#if SALT_MEMORY_DEBUG_DOUBLE_FREE
using Node_free_list  = Free_list;
using Array_free_list = Free_list;
#else
using Node_free_list  = Unordered_free_list;
using Array_free_list = Free_list;
#endif

} // namespace salt::detail
