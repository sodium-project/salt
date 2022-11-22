#pragma once

#include <unordered_map>

#include <salt/memory/heap_allocator.hpp>

struct [[nodiscard]] Memory_info final {
    void*       memory;
    std::size_t size, alignment;
};

// RawAllocator with various security checks
struct [[nodiscard]] Test_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using is_stateful     = std::true_type;
    using memory_info     = Memory_info;

    void* allocate_node(std::size_t size, std::size_t alignment) {
        auto mem        = salt::Heap_allocator{}.allocate_node(size, alignment);
        last_allocated_ = {mem, size, alignment};
        allocated_[mem] = last_allocated_;
        return mem;
    }

    void deallocate_node(void* node, std::size_t size, std::size_t alignment) noexcept {
        ++dealloc_count_;
        auto iter = allocated_.find(node);
        if (iter == allocated_.end() || iter->second.size != size ||
            iter->second.alignment != alignment) {
            last_valid_ = false;
            return;
        } else
            allocated_.erase(iter);
        salt::Heap_allocator{}.deallocate_node(node, size, alignment);
    }

    std::size_t max_node_size() const noexcept {
        return std::size_t(-1);
    }

    bool last_deallocation_valid() noexcept {
        return last_valid_;
    }

    void reset_last_deallocation_valid() noexcept {
        last_valid_ = true;
    }

    memory_info last_allocated() const noexcept {
        return last_allocated_;
    }

    std::size_t no_allocated() const noexcept {
        return allocated_.size();
    }

    std::size_t no_deallocated() const noexcept {
        return dealloc_count_;
    }

    void reset_deallocation_count() noexcept {
        dealloc_count_ = 0u;
    }

private:
    std::unordered_map<void*, memory_info> allocated_;
    memory_info                            last_allocated_;
    std::size_t                            dealloc_count_ = 0u;
    bool                                   last_valid_    = true;
};