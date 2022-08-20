#include <catch2/catch.hpp>

#include <salt/memory/static_allocator.hpp>

using namespace salt;
using namespace salt::detail;

TEST_CASE("salt::Static_allocator", "[salt-memory/static_allocator.hpp]") {
    static_allocator_storage<256> storage;
    Static_allocator              static_allocator{storage};

    auto ptr = static_allocator.allocate_node(sizeof(char), alignof(char));
    REQUIRE(is_aligned(ptr, alignof(char)));
    REQUIRE_FALSE(is_aligned(ptr, max_alignment));

    static_allocator.deallocate_node(ptr, 1, 1);

    for (std::size_t i = 0u; i < 10u; ++i) {
        auto node = static_allocator.allocate_node(i, max_alignment);
        REQUIRE(detail::is_aligned(node, max_alignment));
        static_allocator.deallocate_node(node, i, 1);
    }

    std::vector<void*> nodes;
    for (std::size_t i = 0u; i < 10u; ++i) {
        auto node = static_allocator.allocate_node(i, max_alignment);
        REQUIRE(detail::is_aligned(node, max_alignment));
        nodes.push_back(node);
    }

    for (std::size_t i = 0u; i < 10u; ++i)
        static_allocator.deallocate_node(nodes[i], i, 1);
}
