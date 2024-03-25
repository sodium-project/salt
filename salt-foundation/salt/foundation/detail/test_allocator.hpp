#pragma once
#include <salt/foundation/heap_allocator.hpp>

namespace salt::fdn {

struct memory_info {
    void*       memory;
    std::size_t size, align;
};

template <template <typename...> class Map>
struct test_allocator {
    using allocator_type  = heap_allocator;
    using size_type       = heap_allocator::size_type;
    using difference_type = heap_allocator::difference_type;
    using stateful        = meta::true_type;

    void* allocate_node(size_type size, size_type alignment) noexcept {
        auto mem        = allocator_type{}.allocate_node(size, alignment);
        last_allocated_ = {mem, size, alignment};
        allocated_[mem] = last_allocated_;
        return mem;
    }

    void deallocate_node(void* ptr, size_type size, size_type alignment) noexcept {
        ++dealloc_count_;
        auto it = allocated_.find(ptr);

        bool const miss = it->second.size != size || it->second.align != alignment;
        if (it == allocated_.end() || miss) {
            last_valid_ = false;
            return;
        } else {
            allocated_.erase(it);
        }
        allocator_type{}.deallocate_node(ptr, size, alignment);
    }

    size_type max_node_size() const noexcept {
        return size_type(-1);
    }

    bool valid() noexcept {
        return last_valid_;
    }

    void reset() noexcept {
        last_valid_    = true;
        dealloc_count_ = 0u;
    }

    memory_info last_allocated() const noexcept {
        return last_allocated_;
    }

    size_type allocated_count() const noexcept {
        return allocated_.size();
    }

    size_type deallocated_count() const noexcept {
        return dealloc_count_;
    }

private:
    Map<void*, memory_info> allocated_;
    memory_info             last_allocated_;
    size_type               dealloc_count_ = 0u;
    bool                    last_valid_    = true;
};

} // namespace salt::fdn