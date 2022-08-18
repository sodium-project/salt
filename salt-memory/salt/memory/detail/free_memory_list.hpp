#pragma once
#include <salt/memory/detail/align.hpp>

namespace salt::detail {

struct [[nodiscard]] Free_memory_list final {
    static constexpr auto min_element_size      = sizeof(std::byte*);
    static constexpr auto min_element_alignment = alignof(std::byte*);

    static constexpr std::size_t min_block_size(std::size_t node_size,
                                                std::size_t number_of_nodes) noexcept {
        return (node_size < min_element_size ? min_element_size : node_size) * number_of_nodes;
    }

    explicit Free_memory_list(std::size_t node_size) noexcept;

    Free_memory_list(std::size_t node_size, void* memory, std::size_t size) noexcept;

    Free_memory_list(Free_memory_list&& other) noexcept;
    ~Free_memory_list() noexcept = default;

    Free_memory_list& operator=(Free_memory_list&& other) noexcept;

    void insert(void* memory, std::size_t size) noexcept;

    void* allocate() noexcept;

    void* allocate(std::size_t n) noexcept;

    void deallocate(void* ptr) noexcept;

    void deallocate(void* ptr, std::size_t n) noexcept;

    std::size_t alignment() const noexcept;

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
        return first_ == nullptr;
    }

private:
    void insert_impl(void* memory, std::size_t size) noexcept;

    std::byte*  first_;
    std::size_t node_size_;
    std::size_t capacity_;
};

} // namespace salt::detail
