#include <salt/foundation/memory/allocator_storage.hpp>
#include <salt/foundation/memory/memory_stack.hpp>

#include <catch2/catch.hpp>

struct memory_info {
    void*       memory;
    std::size_t size, align;
};

struct test_allocator {
    using allocator_type  = salt::memory::heap_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using stateful        = salt::meta::true_type;

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
    using allocation_map = std::unordered_map<void*, memory_info>;
    allocation_map allocated_;
    memory_info    last_allocated_;
    size_type      dealloc_count_ = 0u;
    bool           last_valid_    = true;
};

TEST_CASE("salt::memory::memory_stack", "[salt-memory/memory_stack.hpp]") {
    using namespace salt::memory;
    using memory_stack = memory_stack<allocator_reference<test_allocator>>;

    test_allocator allocator;
    memory_stack   stack{memory_stack::min_block_size(100), allocator};
    CHECK(stack.allocator().block_size() == 232u);

    CHECK(allocator.allocated_count() == 1u);
    CHECK(stack.capacity_left()       == 100u);

    auto capacity = stack.capacity_left();
    SECTION("empty unwind") {
        auto marker = stack.top();
        stack.unwind(marker);
        CHECK(capacity <= 100u);
        CHECK(allocator.allocated_count() == 1u);
        CHECK(allocator.deallocated_count() == 0u);
    }

    SECTION("normal allocation/unwind") {
        stack.allocate(10u, 1u);
        CHECK(stack.capacity_left() == capacity - 10 - 2 * detail::debug_fence_size);

        auto marker = stack.top();
        auto memory = stack.allocate(10u, 16u);
        CHECK(is_aligned(memory, 16u));

        stack.unwind(marker);
        CHECK(stack.capacity_left() == capacity - 10 - 2 * detail::debug_fence_size);

        CHECK(stack.allocate(10u, 16u) == memory);
        CHECK(allocator.allocated_count() == 1u);
        CHECK(allocator.deallocated_count() == 0u);
    }

    SECTION("multiple block allocation/unwind") {
        // note: tests are mostly hoping not to get a segfault

        stack.allocate(10, 1);
        auto marker = stack.top();

        auto old_next = stack.next_capacity();

        stack.allocate(100, 1);
        CHECK(stack.next_capacity() > old_next);
        CHECK(allocator.allocated_count() == 2u);
        CHECK(allocator.deallocated_count() == 0u);

        auto m2 = stack.top();
        CHECK(marker < m2);
        stack.allocate(10, 1);
        stack.unwind(m2);
        stack.allocate(20, 1);

        stack.unwind(marker);
        CHECK(allocator.allocated_count() == 2u);
        CHECK(allocator.deallocated_count() == 0u);

        stack.allocate(10, 1);

        stack.shrink_to_fit();
        CHECK(allocator.allocated_count() == 1u);
        CHECK(allocator.deallocated_count() == 1u);
    }

    SECTION("move") {
        auto other  = salt::meta::move(stack);
        auto marker = other.top();
        other.allocate(10, 1);
        CHECK(allocator.allocated_count() == 1u);

        stack = salt::meta::move(other);
        CHECK(allocator.allocated_count() == 1u);
        stack.unwind(marker);
    }

    SECTION("marker comparision") {
        auto m1 = stack.top();
        auto m2 = stack.top();
        CHECK(m1 == m2);

        stack.allocate(1, 1);
        auto m3 = stack.top();
        CHECK(m1 < m3);

        stack.unwind(m2);
        CHECK(stack.top() == m2);
    }

    SECTION("unwinder") {
        auto marker = stack.top();
        {
            memory_stack_unwinder<decltype(stack)> unwind(stack);
            stack.allocate(10, 1);
            CHECK(unwind.will_unwind());
            CHECK(&unwind.stack() == &stack);
            CHECK(unwind.marker() == marker);
        }
        CHECK(stack.top() == marker);

        memory_stack_unwinder<decltype(stack)> unwind(stack);
        stack.allocate(10, 1);
        unwind.unwind();
        CHECK(stack.top() == marker);
        CHECK(unwind.will_unwind());

        {
            memory_stack_unwinder<decltype(stack)> unwind2(stack);
            stack.allocate(10, 1);
            unwind2.release();
            CHECK(!unwind2.will_unwind());
        }
        CHECK(stack.top() > marker);
        marker = stack.top();

        unwind.release(); // need to release
        unwind = memory_stack_unwinder<decltype(stack)>(stack);
        CHECK(unwind.will_unwind());
        CHECK(unwind.marker() == marker);
        CHECK(&unwind.stack() == &stack);
        auto unwind2 = salt::meta::move(unwind);
        CHECK(unwind2.will_unwind());
        CHECK(&unwind2.stack() == &stack);
        CHECK(unwind2.marker() == marker);
        CHECK(!unwind.will_unwind());
    }

    SECTION("overaligned") {
        auto align = 2 * detail::max_alignment;
        auto mem   = stack.allocate(align, align);
        CHECK(is_aligned(mem, align));
    }
}