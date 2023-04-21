#include <catch2/catch.hpp>

#include <salt/memory/allocator_storage.hpp>
#include <salt/memory/memory_stack.hpp>

#include <salt/memory/detail/test_allocator.hpp>

TEST_CASE("salt::Memory_stack", "[salt-memory/memory_stack.hpp]") {
    using namespace salt;
    using Memory_stack = Memory_stack<Allocator_reference<Test_allocator>>;

    Test_allocator allocator;
    Memory_stack   stack{Memory_stack::min_block_size(100), allocator};

    // clang-format off
    REQUIRE(allocator.no_allocated () == 1u);
    REQUIRE(stack.    capacity_left() == 100u);
    // clang-format on

    auto capacity = stack.capacity_left();
    SECTION("empty unwind") {
        auto marker = stack.top();
        stack.unwind(marker);
        REQUIRE(capacity <= 100u);
        REQUIRE(allocator.no_allocated() == 1u);
        REQUIRE(allocator.no_deallocated() == 0u);
    }

    SECTION("normal allocation/unwind") {
        stack.allocate(10u, 1u);
        REQUIRE(stack.capacity_left() == capacity - 10 - 2 * detail::debug_fence_size);

        auto marker = stack.top();
        auto memory = stack.allocate(10u, 16u);
        REQUIRE(detail::is_aligned(memory, 16u));

        stack.unwind(marker);
        REQUIRE(stack.capacity_left() == capacity - 10 - 2 * detail::debug_fence_size);

        REQUIRE(stack.allocate(10u, 16u) == memory);
        REQUIRE(allocator.no_allocated() == 1u);
        REQUIRE(allocator.no_deallocated() == 0u);
    }

    SECTION("multiple block allocation/unwind") {
        // note: tests are mostly hoping not to get a segfault

        stack.allocate(10, 1);
        auto marker = stack.top();

        auto old_next = stack.next_capacity();

        stack.allocate(100, 1);
        REQUIRE(stack.next_capacity() > old_next);
        REQUIRE(allocator.no_allocated() == 2u);
        REQUIRE(allocator.no_deallocated() == 0u);

        auto m2 = stack.top();
        REQUIRE(marker < m2);
        stack.allocate(10, 1);
        stack.unwind(m2);
        stack.allocate(20, 1);

        stack.unwind(marker);
        REQUIRE(allocator.no_allocated() == 2u);
        REQUIRE(allocator.no_deallocated() == 0u);

        stack.allocate(10, 1);

        stack.shrink_to_fit();
        REQUIRE(allocator.no_allocated() == 1u);
        REQUIRE(allocator.no_deallocated() == 1u);
    }

    SECTION("move") {
        auto other  = std::move(stack);
        auto marker = other.top();
        other.allocate(10, 1);
        REQUIRE(allocator.no_allocated() == 1u);

        stack = std::move(other);
        REQUIRE(allocator.no_allocated() == 1u);
        stack.unwind(marker);
    }

    SECTION("marker comparision") {
        auto m1 = stack.top();
        auto m2 = stack.top();
        REQUIRE(m1 == m2);

        stack.allocate(1, 1);
        auto m3 = stack.top();
        REQUIRE(m1 < m3);

        stack.unwind(m2);
        REQUIRE(stack.top() == m2);
    }

    SECTION("unwinder") {
        auto marker = stack.top();
        {
            Memory_stack_unwinder<decltype(stack)> unwind(stack);
            stack.allocate(10, 1);
            REQUIRE(unwind.will_unwind());
            REQUIRE(&unwind.stack() == &stack);
            REQUIRE(unwind.marker() == marker);
        }
        REQUIRE(stack.top() == marker);

        Memory_stack_unwinder<decltype(stack)> unwind(stack);
        stack.allocate(10, 1);
        unwind.unwind();
        REQUIRE(stack.top() == marker);
        REQUIRE(unwind.will_unwind());

        {
            Memory_stack_unwinder<decltype(stack)> unwind2(stack);
            stack.allocate(10, 1);
            unwind2.release();
            REQUIRE(!unwind2.will_unwind());
        }
        REQUIRE(stack.top() > marker);
        marker = stack.top();

        unwind.release(); // need to release
        unwind = Memory_stack_unwinder<decltype(stack)>(stack);
        REQUIRE(unwind.will_unwind());
        REQUIRE(unwind.marker() == marker);
        REQUIRE(&unwind.stack() == &stack);
        auto unwind2 = std::move(unwind);
        REQUIRE(unwind2.will_unwind());
        REQUIRE(&unwind2.stack() == &stack);
        REQUIRE(unwind2.marker() == marker);
        REQUIRE(!unwind.will_unwind());
    }

    SECTION("overaligned") {
        auto align = 2 * detail::max_alignment;
        auto mem   = stack.allocate(align, align);
        REQUIRE(detail::is_aligned(mem, align));
    }
}