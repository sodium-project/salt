#include <catch2/catch.hpp>

#include <salt/memory/detail/fixed_memory_stack.hpp>
#include <salt/memory/static_allocator.hpp>

using namespace salt;
using namespace salt::detail;

TEST_CASE("salt::detail::Fixed_memory_stack", "[salt-memory/fixed_memory_stack.hpp]") {
    Fixed_memory_stack stack;
    REQUIRE(stack.top() == nullptr);

    SECTION("allocate") {
        Static_allocator_storage<1024> memory;
        stack    = Fixed_memory_stack{&memory};
        auto end = stack.top() + 1024;

        REQUIRE(stack.top() == reinterpret_cast<std::byte*>(&memory));

        SECTION("alignment for allocate") {
            auto ptr = stack.allocate(end, 13, 1u);
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, 1u));

            ptr = stack.allocate(end, 10, 2u);
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, 2u));

            ptr = stack.allocate(end, 10, max_alignment);
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, max_alignment));

            ptr = stack.allocate(end, 10, 2 * max_alignment);
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, 2 * max_alignment));
        }
        SECTION("unwind") {
            REQUIRE(stack.allocate(end, 10u, 1u));
            auto diff = std::size_t(stack.top() - reinterpret_cast<std::byte*>(&memory));
            REQUIRE(diff == 2 * debug_fence_size + 10u);

            REQUIRE(stack.allocate(end, 16u, 1u));
            auto diff2 = std::size_t(stack.top() - reinterpret_cast<std::byte*>(&memory));
            REQUIRE(diff2 == 2 * debug_fence_size + 16u + diff);

            stack.unwind(reinterpret_cast<std::byte*>(&memory) + diff);
            REQUIRE(stack.top() == reinterpret_cast<std::byte*>(&memory) + diff);

            auto top = stack.top();
            REQUIRE(!stack.allocate(end, 1024, 1));
            REQUIRE(stack.top() == top);
        }
    }
    SECTION("move") {
        Static_allocator_storage<1024> memory;
        auto                           end = reinterpret_cast<std::byte*>(&memory) + 1024;

        Fixed_memory_stack other(reinterpret_cast<std::byte*>(&memory));
        REQUIRE(other.top() == reinterpret_cast<std::byte*>(&memory));

        stack = std::move(other);
        REQUIRE(stack.top() == reinterpret_cast<std::byte*>(&memory));

        REQUIRE(!other.allocate(end, 10, 1));
        REQUIRE(stack.allocate(end, 10, 1));
        auto top = stack.top();

        other = std::move(stack);
        REQUIRE(other.top() == top);
        REQUIRE(!stack.allocate(end, 10, 1));
        REQUIRE(other.allocate(end, 10, 1));
    }
}
