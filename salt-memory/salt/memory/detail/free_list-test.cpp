#include <catch2/catch.hpp>

#include <salt/memory/detail/free_list.hpp>
#include <salt/memory/static_allocator.hpp>

#include <algorithm>
#include <random>
#include <vector>

using namespace salt;
using namespace salt::detail;

template <typename FreeList> void use_list_node(FreeList& list) {
    std::vector<void*> ptrs;
    auto               capacity = list.capacity();

    // allocate and deallocate in reverse order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto ptr = list.allocate();
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }
        REQUIRE(list.capacity() == 0u);
        REQUIRE(list.empty());

        std::reverse(ptrs.begin(), ptrs.end());

        for (auto p : ptrs)
            list.deallocate(p);
        REQUIRE(list.capacity() == capacity);
        REQUIRE(!list.empty());
        ptrs.clear();
    }

    // allocate and deallocate in same order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto ptr = list.allocate();
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }
        REQUIRE(list.capacity() == 0u);
        REQUIRE(list.empty());

        for (auto p : ptrs)
            list.deallocate(p);
        REQUIRE(list.capacity() == capacity);
        REQUIRE(!list.empty());
        ptrs.clear();
    }

    // allocate and deallocate in random order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto ptr = list.allocate();
            REQUIRE(ptr);
            REQUIRE(is_aligned(ptr, list.alignment()));
            ptrs.push_back(ptr);
        }
        REQUIRE(list.capacity() == 0u);
        REQUIRE(list.empty());

        std::shuffle(ptrs.begin(), ptrs.end(), std::mt19937{});

        for (auto p : ptrs)
            list.deallocate(p);
        REQUIRE(list.capacity() == capacity);
        REQUIRE(!list.empty());
        ptrs.clear();
    }
}

template <typename FreeList> void check_list(FreeList& list, void* memory, std::size_t size) {
    auto old_capacity = list.capacity();

    list.insert(memory, size);
    REQUIRE(!list.empty());
    REQUIRE(list.capacity() <= old_capacity + size / list.node_size());

    old_capacity = list.capacity();

    auto node = list.allocate();
    REQUIRE(node);
    REQUIRE(is_aligned(node, list.alignment()));
    REQUIRE(list.capacity() == old_capacity - 1);

    list.deallocate(node);
    REQUIRE(list.capacity() == old_capacity);

    use_list_node(list);
}

template <typename FreeList> void check_move(FreeList& list) {
    static_allocator_storage<1024> memory;
    list.insert(&memory, 1024);

    auto ptr = list.allocate();
    REQUIRE(ptr);
    REQUIRE(is_aligned(ptr, list.alignment()));
    auto capacity = list.capacity();

    auto list2 = std::move(list);
    REQUIRE(list.empty());
    REQUIRE(list.capacity() == 0u);
    REQUIRE(!list2.empty());
    REQUIRE(list2.capacity() == capacity);

    list2.deallocate(ptr);

    static_allocator_storage<1024> memory2;
    list.insert(&memory2, 1024);
    REQUIRE(!list.empty());
    REQUIRE(list.capacity() <= 1024 / list.node_size());

    ptr = list.allocate();
    REQUIRE(ptr);
    REQUIRE(is_aligned(ptr, list.alignment()));
    list.deallocate(ptr);

    ptr = list2.allocate();

    list = std::move(list2);
    REQUIRE(list2.empty());
    REQUIRE(list2.capacity() == 0u);
    REQUIRE(!list.empty());
    REQUIRE(list.capacity() == capacity);

    list.deallocate(ptr);
}

void use_list_array(Free_list& list) {
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

TEST_CASE("salt::detail::Unordered_free_list", "[salt-memory/free_memory_list.hpp]") {
    Unordered_free_list list(4);
    REQUIRE(list.empty());
    REQUIRE(list.node_size() >= 4);
    REQUIRE(list.capacity() == 0u);

    SECTION("normal insert") {
        static_allocator_storage<1024> memory;
        check_list(list, &memory, 1024);

        check_move(list);
    }
    SECTION("uneven insert") {
        static_allocator_storage<1023> memory; // not dividable
        check_list(list, &memory, 1023);

        check_move(list);
    }
    SECTION("multiple insert") {
        static_allocator_storage<1024> a;
        static_allocator_storage<100>  b;
        static_allocator_storage<1337> c;
        check_list(list, &a, 1024);
        check_list(list, &b, 100);
        check_list(list, &c, 1337);

        check_move(list);
    }
}

TEST_CASE("salt::detail::Free_list", "[salt-memory/free_memory_list.hpp]") {
    Free_list list(4);
    REQUIRE(list.empty());
    REQUIRE(list.node_size() >= 4);
    REQUIRE(list.capacity() == 0u);

    SECTION("normal insert") {
        static_allocator_storage<1024> memory;
        check_list(list, &memory, 1024);
        use_list_array(list);

        check_move(list);
    }
    SECTION("uneven insert") {
        static_allocator_storage<1023> memory; // not dividable
        check_list(list, &memory, 1023);
        use_list_array(list);

        check_move(list);
    }
    SECTION("multiple insert") {
        static_allocator_storage<1024> a;
        static_allocator_storage<100>  b;
        static_allocator_storage<1337> c;
        check_list(list, &a, 1024);
        use_list_array(list);
        check_list(list, &b, 100);
        use_list_array(list);
        check_list(list, &c, 1337);
        use_list_array(list);

        check_move(list);
    }
}
