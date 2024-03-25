#include <salt/foundation.hpp>

#include <catch2/catch.hpp>

TEST_CASE("salt::fdn::memory_pool_list", "[salt-foundation/memory_pool_list.hpp]") {
    using memory_pool_list  = salt::fdn::memory_pool_list<>;
    using allocator_traits  = salt::fdn::allocator_traits<memory_pool_list>;

    const auto       max_size = 16u;
    memory_pool_list pool{max_size, 4000};
    CHECK(allocator_traits::max_node_size (pool) == max_size);
    CHECK(allocator_traits::max_array_size(pool) >= 4000);
    CHECK(allocator_traits::max_alignment (pool) >= 8);
    CHECK(pool.capacity() <= 4000u);
    CHECK(pool.allocator().growth_factor() == 2.0f);

    for (auto node_size = 0u; node_size < max_size; ++node_size) {
        CHECK(pool.capacity(node_size) == 0u);
    }

    SECTION("normal alloc/dealloc") {
        std::vector<void*> a, b;
        for (auto i = 0u; i < 5u; ++i) {
            a.push_back(pool.allocate_node(1));
            b.push_back(pool.try_allocate_node(8));
            CHECK(b.back());
        }
        CHECK(pool.capacity() <= 4000u);

        std::shuffle(a.begin(), a.end(), std::mt19937{});
        std::shuffle(b.begin(), b.end(), std::mt19937{});

        for (auto ptr : a) {
            CHECK(pool.try_deallocate_node(ptr, 1));
        }
        for (auto ptr : b) {
            pool.deallocate_node(ptr, 8);
        }
    }

    SECTION("single array alloc") {        
        auto memory = allocator_traits::allocate_array(pool, 4, 4, 0);
        CHECK(memory);
        allocator_traits::deallocate_array(pool, memory, 4, 4, 0);
    }

    SECTION("array alloc/dealloc") {
        std::vector<void*> a, b;
        for (auto i = 0u; i < 5u; ++i) {
            a.push_back(pool.allocate_array(4, 4));
            b.push_back(pool.try_allocate_array(5, 5));
            CHECK(b.back());
        }
        CHECK(pool.capacity() <= 4000u);

        std::shuffle(a.begin(), a.end(), std::mt19937{});
        std::shuffle(b.begin(), b.end(), std::mt19937{});

        for (auto ptr : a) {
            CHECK(pool.try_deallocate_array(ptr, 4, 4));
        }
        for (auto ptr : b) {
            pool.deallocate_array(ptr, 5, 5);
        }
    }

    SECTION("multiple block alloc/dealloc") {
        std::vector<void*> a, b;
        for (auto i = 0u; i < 1000u; ++i) {
            a.push_back(pool.allocate_node(1));
            b.push_back(pool.allocate_node(5));
        }

        std::shuffle(a.begin(), a.end(), std::mt19937{});
        std::shuffle(b.begin(), b.end(), std::mt19937{});

        for (auto ptr : a) {
            pool.deallocate_node(ptr, 1);
        }
        for (auto ptr : b) {
            pool.deallocate_node(ptr, 5);
        }
    }

    SECTION("move") {
        memory_pool_list new_pool{std::move(pool)};
        CHECK(new_pool.max_node_size() == max_size);
        CHECK(new_pool.capacity() <= 4000u);
        CHECK(new_pool.next_capacity() >= 4000u);

        new_pool.reserve(8, 1);
        auto memory = new_pool.allocate_node(8);
        CHECK(memory);
        new_pool.deallocate_node(memory, 8);
    }

    SECTION("reserve_memory") {
        memory_pool_list small_pool{8, 128};
        CHECK(small_pool.max_node_size() == 8);
        CHECK(small_pool.capacity() <= 128u);
        CHECK(small_pool.next_capacity() >= 128u);

        auto memory = small_pool.allocate_array(6, 8);
        CHECK(memory);
        //small_pool.deallocate_array(memory, 6, 8);
    }

    SECTION("try_reserve_memory array") {
        memory_pool_list small_pool{8, 128};
        CHECK(small_pool.max_node_size() == 8);
        CHECK(small_pool.capacity() <= 128u);
        CHECK(small_pool.next_capacity() >= 128u);

        auto memory = small_pool.try_allocate_array(6, 8);
        CHECK(memory);
        CHECK_FALSE(small_pool.try_allocate_array(1, 1));
        CHECK(small_pool.try_deallocate_array(memory, 6, 8));

        CHECK_FALSE(small_pool.try_allocate_array(16, 8));
        CHECK_FALSE(small_pool.try_allocate_array(1, max_size + 1));
        CHECK_FALSE(small_pool.try_deallocate_array(nullptr, 1, 1));
    }

    SECTION("try_reserve_memory node") {
        memory_pool_list small_pool{8, 128};
        CHECK(small_pool.max_node_size() == 8);
        CHECK(small_pool.capacity() <= 128u);
        CHECK(small_pool.next_capacity() >= 128u);

        std::vector<void*> a;
        for (auto i = 0; i < 6; ++i) {
            auto memory = small_pool.try_allocate_node(8);
            CHECK(memory);
            a.push_back(memory);
        }
        CHECK_FALSE(small_pool.try_allocate_node(1));
        
        CHECK_FALSE(small_pool.try_allocate_node(max_size + 1));
        CHECK_FALSE(small_pool.try_deallocate_node(nullptr, 1));
    }
}