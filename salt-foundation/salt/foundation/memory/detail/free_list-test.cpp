#include <salt/foundation/memory/detail/free_list.hpp>
#include <salt/foundation/memory/static_allocator.hpp>

#include <catch2/catch.hpp>

#include <algorithm>
#include <random>
#include <vector>

using namespace salt::memory;

// clang-format off
template <typename MemoryList>
void use_list_node(MemoryList& list) {
    std::vector<void*> ptrs;
    auto               capacity = list.capacity();

    // allocate and deallocate in reverse order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto* ptr = list.allocate();
            CHECK(ptr);
            CHECK(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }
        CHECK(list.capacity() == 0u);
        CHECK(list.empty());

        std::reverse(ptrs.begin(), ptrs.end());

        for (auto p : ptrs)
            list.deallocate(p);
        CHECK(list.capacity() == capacity);
        CHECK(!list.empty());
        ptrs.clear();
    }

    // allocate and deallocate in same order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto* ptr = list.allocate();
            CHECK(ptr);
            CHECK(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }
        CHECK(list.capacity() == 0u);
        CHECK(list.empty());

        for (auto p : ptrs)
            list.deallocate(p);
        CHECK(list.capacity() == capacity);
        CHECK(!list.empty());
        ptrs.clear();
    }

    // allocate and deallocate in random order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto* ptr = list.allocate();
            CHECK(ptr);
            CHECK(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }
        CHECK(list.capacity() == 0u);
        CHECK(list.empty());

        std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

        for (auto p : ptrs)
            list.deallocate(p);
        CHECK(list.capacity() == capacity);
        CHECK(!list.empty());
        ptrs.clear();
    }
}

template <typename MemoryList>
void check_list(MemoryList& list, void* memory, std::size_t size) {
    auto old_capacity = list.capacity();

    list.insert(memory, size);
    CHECK(!list.empty());
    CHECK(list.capacity() <= old_capacity + size / list.node_size());

    old_capacity = list.capacity();

    auto* node = list.allocate();
    CHECK(node);
    CHECK(is_aligned(node, list.alignment()));
    CHECK(list.capacity() == old_capacity - 1);

    list.deallocate(node);
    CHECK(list.capacity() == old_capacity);

    use_list_node(list);
}

template <typename MemoryList>
void check_move(MemoryList& list) {
    static_allocator_storage<1024> memory;
    list.insert(&memory, 1024);

    auto* ptr = list.allocate();
    CHECK(ptr);
    CHECK(is_aligned(ptr, list.alignment()));
    auto capacity = list.capacity();

    auto list2 = std::move(list);
    CHECK(list.empty());
    CHECK(list.capacity() == 0u);
    CHECK(!list2.empty());
    CHECK(list2.capacity() == capacity);

    MemoryList list3(4);
    list3 = std::move(list2);
    CHECK(list2.empty());
    CHECK(list2.capacity() == 0u);
    CHECK(!list3.empty());
    CHECK(list3.capacity() == capacity);

    list3.deallocate(ptr);
}

void use_list_array(detail::ordered_free_list& list) {
    std::vector<void*> ptrs;
    auto               capacity = list.capacity();
    // We would need capacity / 3 nodes, but the memory might not be contiguous.
    auto number_of_allocations = capacity / 6;

    // allocate and deallocate in reverse order
    {
        for (std::size_t i = 0u; i != number_of_allocations; ++i) {
            auto ptr = list.allocate(3 * list.node_size());
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }

        std::reverse(ptrs.begin(), ptrs.end());

        for (auto p : ptrs)
            list.deallocate(p, 3 * list.node_size());
        REQUIRE(list.capacity() == capacity);
        ptrs.clear();
    }

    // allocate and deallocate in same order
    {
        for (std::size_t i = 0u; i != number_of_allocations; ++i) {
            auto ptr = list.allocate(3 * list.node_size());
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }

        for (auto p : ptrs)
            list.deallocate(p, 3 * list.node_size());
        REQUIRE(list.capacity() == capacity);
        REQUIRE(!list.empty());
        ptrs.clear();
    }

    // allocate and deallocate in random order
    {
        for (std::size_t i = 0u; i != number_of_allocations; ++i) {
            auto ptr = list.allocate(3 * list.node_size());
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }

        std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

        for (auto p : ptrs)
            list.deallocate(p, 3 * list.node_size());
        REQUIRE(list.capacity() == capacity);
        REQUIRE(!list.empty());
        ptrs.clear();
    }
}
// clang-format on

TEST_CASE("salt::memory::detail::free_list", "[salt-memory/free_list.hpp]") {
    SECTION("construct") {
        detail::free_list list(4);
        REQUIRE(list.empty());
        REQUIRE(list.node_size() >= 4);
        REQUIRE(list.capacity() == 0u);
    }
    SECTION("normal insert") {
        static_allocator_storage<1024> memory;
        detail::free_list       list(4);
        check_list(list, &memory, 1024);

        check_move(list);
    }
    SECTION("uneven insert") {
        static_allocator_storage<1023> memory; // not dividable
        detail::free_list       list(4);
        check_list(list, &memory, 1023);

        check_move(list);
    }
    SECTION("multiple insert") {
        static_allocator_storage<1024> a;
        static_allocator_storage<100>  b;
        static_allocator_storage<1337> c;
        detail::free_list       list(4);

        check_list(list, &a, 1024);
        check_list(list, &b, 100);
        check_list(list, &c, 1337);

        check_move(list);
    }
}

TEST_CASE("salt::memory::detail::ordered_free_list", "[salt-memory/free_list.hpp]") {
    SECTION("construct") {
        detail::ordered_free_list list(4);
        REQUIRE(list.empty());
        REQUIRE(list.node_size() >= 4);
        REQUIRE(list.capacity() == 0u);
    }

    SECTION("normal insert") {
        static_allocator_storage<1024>   memory;
        detail::ordered_free_list list(4);
        check_list(list, &memory, 1024);
        use_list_array(list);

        check_move(list);
    }
    SECTION("uneven insert") {
        static_allocator_storage<1023>   memory; // not dividable
        detail::ordered_free_list list(4);
        check_list(list, &memory, 1023);
        use_list_array(list);

        check_move(list);
    }
    SECTION("multiple insert") {
        static_allocator_storage<1024>   a;
        static_allocator_storage<100>    b;
        static_allocator_storage<1337>   c;
        detail::ordered_free_list list(4);

        check_list(list, &a, 1024);
        use_list_array(list);
        check_list(list, &b, 100);
        use_list_array(list);
        check_list(list, &c, 1337);
        use_list_array(list);

        check_move(list);
    }
}