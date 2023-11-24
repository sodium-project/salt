#include <salt/foundation/memory/std_allocator_adapter.hpp>
#include <salt/foundation/memory/temporary_allocator.hpp>

#include <catch2/catch.hpp>

using namespace salt;

template <typename T>
using stack_vector = std::vector<T, memory::std_allocator_adapter<T, memory::temporary_allocator>>;

TEST_CASE("salt::memory::temporary_allocator", "[salt-memory/temporary_allocator.hpp]") {

    SECTION("test temporary allocator") {
        memory::temporary_allocator allocator;

        auto ptr = allocator.allocate(sizeof(char), alignof(char));
        INFO("ptr = " << ptr << ", max_alignment = " << memory::detail::max_alignment);
        CHECK(memory::is_aligned(ptr, alignof(char)));
        CHECK_FALSE(memory::is_aligned(ptr, memory::detail::max_alignment));
    }

    SECTION("test temporary vector") {
        memory::temporary_allocator allocator;

        stack_vector<int> v{allocator};
        v.push_back(2023);
        CHECK(2023 == v[0]);
    }
}