#include <catch2/catch.hpp>

#include <algorithm>
#include <random>
#include <vector>

#include <salt/memory/memory_pool.hpp>

using namespace salt;
using namespace salt::detail;

TEST_CASE("salt::Memory_pool", "[salt-memory/memory_pool.hpp]") {
    using memory_pool = Memory_pool<>;
    {
        memory_pool pool{4, memory_pool::min_block_size(4, 25)};
        REQUIRE(pool.node_size() >= 4u);
        REQUIRE(pool.capacity() >= 25 * 4u);
        REQUIRE(pool.size() >= 25 * 4u);

        SECTION("normal alloc/dealloc") {
            std::vector<void*> ptrs;
            auto               capacity = pool.capacity();
            REQUIRE(capacity / 4 >= 25);
            for (std::size_t i = 0u; i < 25; ++i)
                ptrs.push_back(pool.allocate_node());
            REQUIRE(pool.capacity() >= 0u);

            std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

            for (auto ptr : ptrs)
                pool.deallocate_node(ptr);
            REQUIRE(pool.capacity() == capacity);
        }
        SECTION("multiple block alloc/dealloc") {
            std::vector<void*> ptrs;
            auto               capacity = pool.capacity();
            for (std::size_t i = 0u; i < capacity / pool.node_size(); ++i)
                ptrs.push_back(pool.allocate_node());
            REQUIRE(pool.capacity() >= 0u);

            ptrs.push_back(pool.allocate_node());
            REQUIRE(pool.capacity() >= capacity - pool.node_size());

            std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

            for (auto ptr : ptrs)
                pool.deallocate_node(ptr);
            REQUIRE(pool.capacity() >= capacity);
        }
    }
    {
        memory_pool pool{16, memory_pool::min_block_size(16, 1)};
        REQUIRE(pool.node_size() == 16u);
        REQUIRE(pool.capacity() == 16u);
        REQUIRE(pool.size() >= 16u);

        auto ptr = pool.allocate_node();
        REQUIRE(ptr);

        pool.deallocate_node(ptr);
    }
}

namespace {
template <typename PoolType>
void use_min_block_size(std::size_t node_size, std::size_t number_of_nodes) {
    auto min_size = Memory_pool<PoolType>::min_block_size(node_size, number_of_nodes);
    Memory_pool<PoolType> pool{node_size, min_size};
    CHECK(pool.capacity() >= node_size * number_of_nodes);

    // First allocations should not require realloc.
    for (auto i = 0u; i < number_of_nodes; ++i) {
        auto ptr = pool.try_allocate_node();
        CHECK(ptr);
    }

    // Further allocation might require it, but should still succeed then.
    auto ptr = pool.allocate_node();
    CHECK(ptr);
}
} // namespace

TEST_CASE("salt::Memory_pool::min_block_size", "[salt-memory/memory_pool.hpp]") {
    SECTION("node pool") {
        use_min_block_size<Node_pool>(1, 1);
        use_min_block_size<Node_pool>(16, 1);
        use_min_block_size<Node_pool>(1, 1000);
        use_min_block_size<Node_pool>(16, 1000);
    }
    SECTION("array pool") {
        use_min_block_size<Array_pool>(1, 1);
        use_min_block_size<Array_pool>(16, 1);
        use_min_block_size<Array_pool>(1, 1000);
        use_min_block_size<Array_pool>(16, 1000);
    }
}