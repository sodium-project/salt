#include <salt/foundation/memory/memory_pool.hpp>

#include <catch2/catch.hpp>

TEST_CASE("salt::memory::memory_pool", "[salt-memory/memory_pool.hpp]") {
    using memory_pool       = salt::memory::memory_pool<>;
    using allocator_traits  = salt::memory::allocator_traits<memory_pool>;
    using composable_traits = salt::memory::composable_traits<memory_pool>;
    CHECK(memory_pool::min_node_size == 8);
    {
        memory_pool pool{4, memory_pool::min_block_size(4, 25)};
        CHECK(allocator_traits::max_node_size (pool) >= 4);
        CHECK(allocator_traits::max_array_size(pool) >= 25 * 4);
        CHECK(allocator_traits::max_alignment (pool) == 8);
        CHECK(pool.capacity() >= 25 * 4);
        CHECK(pool.allocator().growth_factor() == 2.0f);

        SECTION("normal alloc/dealloc") {
            std::vector<void*> ptrs;
            auto               capacity = pool.capacity();
            CHECK(capacity / 4 >= 25);
            for (std::size_t i = 0u; i < 25; ++i) {
                ptrs.push_back(allocator_traits::allocate_node(pool, 0, 0));
            }
            CHECK(pool.capacity() >= 0u);

            std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

            for (auto ptr : ptrs) {
                allocator_traits::deallocate_node(pool, ptr, 0, 0);
            }
            CHECK(pool.capacity() == capacity);
        }

        SECTION("try alloc/dealloc") {
            std::vector<void*> ptrs;
            auto               capacity = pool.capacity();
            CHECK(capacity / 4 >= 25);
            for (std::size_t i = 0u; i < 25; ++i) {
                auto memory = composable_traits::try_allocate_node(pool, 0, 0);
                CHECK(memory);
                ptrs.push_back(memory);
            }
            CHECK_FALSE(pool.try_allocate_node());
            CHECK(pool.capacity() >= 0u);

            for (auto ptr : ptrs) {
                CHECK(composable_traits::try_deallocate_node(pool, ptr, 0, 0));
            }
            CHECK_FALSE(pool.try_deallocate_node(nullptr));
            CHECK(pool.capacity() == capacity);
        }

        SECTION("multiple block alloc/dealloc") {
            std::vector<void*> ptrs;
            auto               capacity = pool.capacity();
            for (std::size_t i = 0u; i < capacity / pool.node_size(); ++i) {
                ptrs.push_back(pool.allocate_node());
            }
            CHECK(pool.capacity() >= 0u);

            ptrs.push_back(pool.allocate_node());
            CHECK(pool.capacity() >= capacity - pool.node_size());

            std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

            for (auto ptr : ptrs) {
                pool.deallocate_node(ptr);
            }
            CHECK(pool.capacity() >= capacity);
        }

        SECTION("move") {
            memory_pool new_pool{salt::meta::move(pool)};
            CHECK(new_pool.node_size() >= 4u);
            CHECK(new_pool.capacity() >= 25 * 4u);
            CHECK(new_pool.next_capacity() >= 25 * 4u);

            auto* ptr = new_pool.allocate_node();
            CHECK(ptr);
            new_pool.deallocate_node(ptr);
        }
    }
    {
        memory_pool pool{16, memory_pool::min_block_size(16, 1)};
        CHECK(pool.node_size() == 16u);
        CHECK(pool.capacity() == 16u);
        CHECK(pool.next_capacity() >= 16u);

        auto* ptr = pool.allocate_node();
        CHECK(ptr);

        pool.deallocate_node(ptr);
    }
}

TEST_CASE("salt::memory::memory_pool_array", "[salt-memory/memory_pool.hpp]") {
    using memory_pool      = salt::memory::memory_pool<salt::memory::array_pool>;
    using allocator_traits = salt::memory::allocator_traits<memory_pool>;

    memory_pool pool{4, memory_pool::min_block_size(4, 25)};
    CHECK(pool.node_size() >= 4u);
    CHECK(pool.capacity() >= 25 * 4u);
    CHECK(pool.next_capacity() >= 25 * 4u);

    SECTION("normal alloc/dealloc") {
        auto capacity = pool.capacity();
        CHECK(capacity / 4 >= 25);

        auto* array = pool.allocate_array(25);
        CHECK(pool.capacity() == 0u);

        pool.deallocate_array(array, 25);
        CHECK(pool.capacity() == capacity);
    }

    SECTION("try alloc/dealloc") {
        auto capacity = pool.capacity();
        CHECK(capacity / 4 >= 25);

        auto* array = pool.try_allocate_array(25);
        CHECK(array);
        CHECK(pool.capacity() == 0u);
        CHECK_FALSE(pool.try_allocate_array(5));

        CHECK(pool.try_deallocate_array(array, 25));
        CHECK_FALSE(pool.try_deallocate_array(nullptr, 25));
        CHECK(pool.capacity() == capacity);
    }

    SECTION("allocate_array small") {
        memory_pool small_pool{memory_pool::min_node_size, memory_pool::min_block_size(1, 1)};
        auto* array = allocator_traits::allocate_array(small_pool, 3, small_pool.node_size(), 0);
        CHECK(array);
        allocator_traits::deallocate_array(small_pool, array, 3, small_pool.node_size(), 0);
    }
}

namespace {
template <typename PoolType>
void use_min_block_size(std::size_t node_size, std::size_t number_of_nodes) {
    using memory_pool = salt::memory::memory_pool<PoolType>;

    auto        min_size = memory_pool::min_block_size(node_size, number_of_nodes);
    memory_pool pool{node_size, min_size};
    CHECK(pool.capacity() >= node_size * number_of_nodes);

    // First allocations should not require realloc.
    for (auto i = 0u; i < number_of_nodes; ++i) {
        auto* ptr = pool.try_allocate_node();
        CHECK(ptr);
    }

    // Further allocation might require it, but should still succeed then.
    auto* ptr = pool.allocate_node();
    CHECK(ptr);
}
} // namespace

TEST_CASE("salt::memory::memory_pool::min_block_size", "[salt-memory/memory_pool.hpp]") {
    SECTION("node pool") {
        use_min_block_size<salt::memory::node_pool>(1, 1);
        use_min_block_size<salt::memory::node_pool>(16, 1);
        use_min_block_size<salt::memory::node_pool>(1, 1000);
        use_min_block_size<salt::memory::node_pool>(16, 1000);
    }

    SECTION("array pool") {
        use_min_block_size<salt::memory::array_pool>(1, 1);
        use_min_block_size<salt::memory::array_pool>(16, 1);
        use_min_block_size<salt::memory::array_pool>(1, 1000);
        use_min_block_size<salt::memory::array_pool>(16, 1000);
    }
}