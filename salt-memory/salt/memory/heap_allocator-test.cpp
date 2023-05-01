#include <catch2/catch.hpp>
#include <vector>

#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/mimalloc_allocator.hpp>
#include <salt/memory/heap_allocator.hpp>

using namespace salt;
using namespace salt::detail;

template <class Allocator>
void check_default_allocator(Allocator& allocator, std::size_t alignment = detail::max_alignment) {
    auto* ptr = allocator.allocate_node(1, 1);
    REQUIRE(detail::is_aligned(ptr, alignment));

    allocator.deallocate_node(ptr, 1, 1);

    for (std::size_t i = 0u; i != 10u; ++i) {
        auto* node = allocator.allocate_node(i, 1);
        REQUIRE(detail::is_aligned(node, alignment));
        allocator.deallocate_node(node, i, 1);
    }

    std::vector<void*> nodes;
    for (std::size_t i = 0u; i != 10u; ++i) {
        auto* node = allocator.allocate_node(i, 1);
        REQUIRE(detail::is_aligned(node, alignment));
        nodes.push_back(node);
    }

    for (std::size_t i = 0u; i != 10u; ++i)
        allocator.deallocate_node(nodes[i], i, 1);
}

TEST_CASE("salt::Heap_allocator", "[salt-memory/heap_allocator.hpp]") {
    Heap_allocator allocator;
    check_default_allocator(allocator);
}