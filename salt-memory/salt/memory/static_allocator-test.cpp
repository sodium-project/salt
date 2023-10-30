#include <salt/memory/static_allocator.hpp>

#include <catch2/catch.hpp>

using namespace salt::memory;

TEST_CASE("salt::memory::static_allocator", "[salt-memory/static_allocator.hpp]") {
    static_allocator_storage<1024> storage;
    static_allocator               allocator{storage};
    CHECK(allocator.max_node_size() == 1024);
    CHECK(allocator.max_alignment() == 0xffffffffffffffff);

    auto* ptr = allocator.allocate_node(sizeof(char), alignof(char));
    CHECK(is_aligned(ptr, alignof(char)));
    CHECK_FALSE(is_aligned(ptr, max_alignment));
    allocator.deallocate_node(ptr, 1, 1);

    std::vector<void*> nodes;
    for (std::size_t i = 0u; i < 8u; ++i) {
        auto* node = allocator.allocate_node(i, max_alignment);
        CHECK(is_aligned(node, max_alignment));
        nodes.push_back(node);
    }

    for (std::size_t i = 0u; i < 8u; ++i) {
        allocator.deallocate_node(nodes[i], i, 1);
    }
}

TEST_CASE("salt::memory::static_block_allocator", "[salt-memory/static_allocator.hpp]") {
    static_allocator_storage<1024> storage;
    std::size_t const              block_size = 8;
    static_block_allocator         allocator{block_size, storage};
    CHECK(allocator.block_size() == block_size);

    for (std::size_t i = 0u; i < 8u; ++i) {
        auto block = allocator.allocate_block();
        CHECK(block.memory != nullptr);
        CHECK(block.size == block_size);
        allocator.deallocate_block(block);
    }

    static_block_allocator other{salt::meta::move(allocator)};
    for (std::size_t i = 0u; i < 10u; ++i) {
        auto block = other.allocate_block();
        CHECK(block.memory != nullptr);
        CHECK(block.size == block_size);
        other.deallocate_block(block);
    }
    
    static_block_allocator allocator2{16, storage};
    allocator2 = salt::meta::move(other);
    for (std::size_t i = 0u; i < 5u; ++i) {
        auto block = allocator2.allocate_block();
        CHECK(block.memory != nullptr);
        CHECK(block.size == block_size);
        allocator2.deallocate_block(block);
    }
}