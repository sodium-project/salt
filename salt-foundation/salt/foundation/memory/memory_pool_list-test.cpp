#include <salt/foundation/memory/memory_pool_list.hpp>

#include <catch2/catch.hpp>

TEST_CASE("salt::memory::memory_pool_list", "[salt-memory/memory_pool_list.hpp]") {
    using memory_pool_list    = salt::memory::memory_pool_list<>;
    const auto       max_size = 16u;
    memory_pool_list pool{max_size, 4000};
    CHECK(pool.max_node_size() == max_size);
    CHECK(pool.capacity() <= 4000u);
    CHECK(pool.size() >= 4000u);
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
        auto memory = pool.allocate_array(4, 4);
        CHECK(memory);
        pool.deallocate_array(memory, 4, 4);
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
        memory_pool_list new_pool{salt::meta::move(pool)};
        CHECK(new_pool.max_node_size() == max_size);
        CHECK(new_pool.capacity() <= 4000u);
        CHECK(new_pool.size() >= 4000u);

        new_pool.reserve(8, 1);
        auto memory = new_pool.allocate_node(8);
        CHECK(memory);
        new_pool.deallocate_node(memory, 8);
    }
}