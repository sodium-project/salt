#include <catch2/catch.hpp>

#include <salt/memory/containers.hpp>
#include <salt/memory/memory_pool.hpp>
#include <salt/memory/smart_ptr.hpp>
#include <salt/memory/static_allocator.hpp>
#include <salt/memory/temporary_allocator.hpp>

struct [[nodiscard]] Dummy_allocator final {
    using allocator_type  = Dummy_allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline std::size_t size = 0;

    void* allocate_node(std::size_t s, std::size_t) {
        size = s;
        return ::operator new(size);
    }

    void deallocate_node(void* ptr, std::size_t, std::size_t) {
        ::operator delete(ptr);
    }
};

TEST_CASE("salt::memory::list", "[salt-memory/containers.hpp]") {
    using namespace salt;
    using namespace salt::literals;

    SECTION("stateless") {
        memory::list<int, Dummy_allocator> list(Dummy_allocator{});
        list.push_back(2023);
        list.push_back(2022);
        list.push_back(2021);

        auto expected_values = std::array{2023, 2022, 2021};
        auto i               = std::size_t{0};
        for (auto value : list) {
            CHECK(expected_values[i++] == value);
        }
        REQUIRE((Dummy_allocator::size <= memory::list_node_size<int>::value));
    }

    SECTION("stateful") {
        using memory_pool = Memory_pool<>;

        memory_pool pool(memory::list_node_size<int>::value, 1_KiB);

        memory::list<int, memory_pool> pool_list(pool);
        pool_list.push_back(2023);
        pool_list.push_back(2022);
        pool_list.push_back(2021);

        auto expected_values = std::array{2023, 2022, 2021};
        auto i               = std::size_t{0};
        for (auto value : pool_list) {
            CHECK(expected_values[i++] == value);
        }
    }
}

TEST_CASE("salt::memory::vector", "[salt-memory/containers.hpp]") {
    using namespace salt;
    using namespace salt::literals;

    SECTION("stateless") {
        Dummy_allocator::size = 0;
        memory::vector<int, Dummy_allocator> vector(Dummy_allocator{});
        vector.push_back(2023);
        vector.push_back(2022);
        vector.push_back(2021);

        auto expected_values = std::array{2023, 2022, 2021};
        auto i               = std::size_t{0};
        for (auto value : vector) {
            CHECK(expected_values[i++] == value);
        }
        REQUIRE(Dummy_allocator::size <= 16);
    }

    SECTION("stateful") {
        using static_storage = Static_allocator_storage<1_KiB>;
        using static_pool    = Memory_pool<Node_pool, Static_block_allocator>;

        struct [[nodiscard]] Dummy final {
            double x = 0.0;
            double y = 0.0;

            constexpr bool operator==(Dummy const&) const noexcept = default;
        };

        static_storage storage;
        static_pool    pool(sizeof(Dummy), 1_KiB, storage);

        memory::vector<Dummy, static_pool> static_vector(pool);
        static_vector.push_back({20.0, 23.0});
        static_vector.push_back({21.0, 22.0});
        static_vector.push_back({22.0, 21.0});

        auto expected_values = std::array{Dummy{20.0, 23.0}, Dummy{21.0, 22.0}, Dummy{22.0, 21.0}};
        auto i               = std::size_t{0};
        for (auto value : static_vector) {
            CHECK(expected_values[i++] == value);
        }
    }
}