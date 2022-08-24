#include <catch2/catch.hpp>

#include <salt/memory/detail/free_list.hpp>
#include <salt/memory/detail/free_list_array.hpp>
#include <salt/memory/static_allocator.hpp>

using namespace salt;
using namespace salt::detail;

using free_list   = Unordered_free_list;
using log2_policy = Log2_access_policy;

TEST_CASE("salt::detail::Log2_access_policy", "[salt-memory/Free_list_array.hpp]") {
    REQUIRE(log2_policy::index_from_size(1) == 0u);
    REQUIRE(log2_policy::index_from_size(2) == 1u);
    REQUIRE(log2_policy::index_from_size(3) == 2u);
    REQUIRE(log2_policy::index_from_size(4) == 2u);
    REQUIRE(log2_policy::index_from_size(5) == 3u);
    REQUIRE(log2_policy::index_from_size(6) == 3u);
    REQUIRE(log2_policy::index_from_size(8) == 3u);
    REQUIRE(log2_policy::index_from_size(9) == 4u);

    REQUIRE(log2_policy::size_from_index(0) == 1u);
    REQUIRE(log2_policy::size_from_index(1) == 2u);
    REQUIRE(log2_policy::size_from_index(2) == 4u);
    REQUIRE(log2_policy::size_from_index(3) == 8u);
}

TEST_CASE("salt::detail::Free_list_array", "[salt-memory/Free_list_array.hpp]") {
    Static_allocator_storage<1024> memory;
    Fixed_memory_stack             stack(&memory);

    SECTION("non power of two max size, normal list") {
        using free_list_array = Free_list_array<free_list, log2_policy>;
        free_list_array array(stack, stack.top() + 1024, 15);
        REQUIRE(array.max_node_size() == 16u);
        REQUIRE(array.size() <= 5u);

        REQUIRE(array[1u].node_size() == free_list::min_element_size);
        REQUIRE(array[2u].node_size() == free_list::min_element_size);
        REQUIRE(array[9u].node_size() == 16u);
        REQUIRE(array[15u].node_size() == 16u);
    }
}