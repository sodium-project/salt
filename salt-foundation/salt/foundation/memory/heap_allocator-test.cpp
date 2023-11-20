#include <salt/foundation/memory/align.hpp>
#include <salt/foundation/memory/heap_allocator.hpp>

#include <catch2/catch.hpp>
#include <vector>

using namespace salt::memory;

template <typename Allocator>
void check_default_allocator(Allocator& allocator, std::size_t alignment = detail::max_alignment) {
    auto* ptr = allocator.allocate_node(1, 1);
    CHECK(is_aligned(ptr, alignment));

    allocator.deallocate_node(ptr, 1, 1);

    for (std::size_t i = 0u; i != 10u; ++i) {
        auto* node = allocator.allocate_node(i, 1);
        CHECK(is_aligned(node, alignment));
        allocator.deallocate_node(node, i, 1);
    }

    std::vector<void*> nodes;
    for (std::size_t i = 0u; i != 10u; ++i) {
        auto* node = allocator.allocate_node(i, 1);
        CHECK(is_aligned(node, alignment));
        nodes.push_back(node);
    }

    for (std::size_t i = 0u; i != 10u; ++i) {
        allocator.deallocate_node(nodes[i], i, 1);
    }
}

TEST_CASE("salt::memory::heap_allocator", "[salt-memory/heap_allocator.hpp]") {
    heap_allocator allocator;
    check_default_allocator(allocator);
}