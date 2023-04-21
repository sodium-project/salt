#include <catch2/catch.hpp>

#include <salt/memory/static_allocator.hpp>

TEST_CASE("salt::Static_allocator", "[salt-memory/static_allocator.hpp]") {
    salt::Static_allocator_storage<256> storage;
    salt::Static_allocator              static_allocator{storage};

    auto ptr = static_allocator.allocate_node(sizeof(char), alignof(char));
    REQUIRE(salt::detail::is_aligned(ptr, alignof(char)));
    REQUIRE_FALSE(salt::detail::is_aligned(ptr, salt::detail::max_alignment));

    static_allocator.deallocate_node(ptr, 1, 1);

    for (std::size_t i = 0u; i < 10u; ++i) {
        auto node = static_allocator.allocate_node(i, salt::detail::max_alignment);
        REQUIRE(salt::detail::is_aligned(node, salt::detail::max_alignment));
        static_allocator.deallocate_node(node, i, 1);
    }

    std::vector<void*> nodes;
    for (std::size_t i = 0u; i < 10u; ++i) {
        auto node = static_allocator.allocate_node(i, salt::detail::max_alignment);
        REQUIRE(salt::detail::is_aligned(node, salt::detail::max_alignment));
        nodes.push_back(node);
    }

    for (std::size_t i = 0u; i < 10u; ++i)
        static_allocator.deallocate_node(nodes[i], i, 1);
}
