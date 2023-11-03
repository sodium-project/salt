#include <salt/memory/detail/fixed_stack.hpp>
#include <salt/memory/static_allocator.hpp>

#include <catch2/catch.hpp>

using namespace salt::memory;

TEST_CASE("salt::memory::detail::fixed_stack", "[salt-memory/fixed_stack.hpp]") {
    detail::fixed_stack stack;
    REQUIRE(stack.top() == nullptr);

    SECTION("allocate") {
        static_allocator_storage<1024> memory;
        stack    = detail::fixed_stack{&memory};
        auto end = stack.top() + 1024;

        CHECK(stack.top() == reinterpret_cast<std::byte*>(&memory));

        SECTION("alignment for allocate") {
            auto* ptr = stack.allocate(end, 13, 1u);
            CHECK(ptr);
            CHECK(is_aligned(ptr, 1u));

            ptr = stack.allocate(end, 10, 2u);
            CHECK(ptr);
            CHECK(is_aligned(ptr, 2u));

            ptr = stack.allocate(end, 10, max_alignment);
            CHECK(ptr);
            CHECK(is_aligned(ptr, max_alignment));

            ptr = stack.allocate(end, 10, 2 * max_alignment);
            CHECK(ptr);
            CHECK(is_aligned(ptr, 2 * max_alignment));
        }

        SECTION("unwind") {
            CHECK(stack.allocate(end, 10u, 1u));
            auto diff = std::size_t(stack.top() - reinterpret_cast<std::byte*>(&memory));
            CHECK(diff == 2 * detail::debug_fence_size + 10u);

            CHECK(stack.allocate(end, 16u, 1u));
            auto diff2 = std::size_t(stack.top() - reinterpret_cast<std::byte*>(&memory));
            CHECK(diff2 == 2 * detail::debug_fence_size + 16u + diff);

            stack.unwind(reinterpret_cast<std::byte*>(&memory) + diff);
            CHECK(stack.top() == reinterpret_cast<std::byte*>(&memory) + diff);

            auto* top = stack.top();
            CHECK(!stack.allocate(end, 1024, 1));
            CHECK(stack.top() == top);
        }
    }

    SECTION("move") {
        static_allocator_storage<1024> memory;
        auto                           end = reinterpret_cast<std::byte*>(&memory) + 1024;

        detail::fixed_stack other(reinterpret_cast<std::byte*>(&memory));
        CHECK(other.top() == reinterpret_cast<std::byte*>(&memory));

        stack = salt::meta::move(other);
        CHECK(stack.top() == reinterpret_cast<std::byte*>(&memory));

        CHECK(!other.allocate(end, 10, 1));
        CHECK(stack.allocate(end, 10, 1));
        auto* top = stack.top();

        other = salt::meta::move(stack);
        CHECK(other.top() == top);
        CHECK(!stack.allocate(end, 10, 1));
        CHECK(other.allocate(end, 10, 1));
    }
}