#include <catch2/catch.hpp>

#include <salt/memory/detail/memory_list.hpp>
#include <salt/memory/static_allocator.hpp>

#include <algorithm>
#include <random>
#include <vector>

using namespace salt;
using namespace salt::detail;

template <typename MemoryList> void use_list_node(MemoryList& list) {
    std::vector<void*> ptrs;
    auto               capacity = list.capacity();

    // allocate and deallocate in reverse order
    {
        for (std::size_t i = 0u; i != capacity; ++i) {
            auto* ptr = list.allocate();
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
            auto* ptr = list.allocate();
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
            auto* ptr = list.allocate();
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

template <typename MemoryList> void check_list(MemoryList& list, void* memory, std::size_t size) {
    auto old_capacity = list.capacity();

    list.insert(memory, size);
    REQUIRE(!list.empty());
    REQUIRE(list.capacity() <= old_capacity + size / list.node_size());

    old_capacity = list.capacity();

    auto* node = list.allocate();
    REQUIRE(node);
    REQUIRE(is_aligned(node, list.alignment()));
    REQUIRE(list.capacity() == old_capacity - 1);

    list.deallocate(node);
    REQUIRE(list.capacity() == old_capacity);

    use_list_node(list);
}

template <typename MemoryList> void check_move(MemoryList& list) {
    Static_allocator_storage<1024> memory;
    list.insert(&memory, 1024);

    auto* ptr = list.allocate();
    REQUIRE(ptr);
    REQUIRE(is_aligned(ptr, list.alignment()));
    auto capacity = list.capacity();

    auto list2 = std::move(list);
    REQUIRE(list.empty());
    REQUIRE(list.capacity() == 0u);
    REQUIRE(!list2.empty());
    REQUIRE(list2.capacity() == capacity);

    list2.deallocate(ptr);
}

void use_list_array(Memory_list& list) {
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

TEST_CASE("salt::detail::Unordered_memory_list", "[salt-memory/memory_list.hpp]") {
    SECTION("construct") {
        Unordered_memory_list list(4);
        REQUIRE(list.empty());
        REQUIRE(list.node_size() >= 4);
        REQUIRE(list.capacity() == 0u);
    }
    SECTION("normal insert") {
        Static_allocator_storage<1024> memory;
        Unordered_memory_list          list(4);
        check_list(list, &memory, 1024);

        check_move(list);
    }
    SECTION("uneven insert") {
        Static_allocator_storage<1023> memory; // not dividable
        Unordered_memory_list          list(4);
        check_list(list, &memory, 1023);

        check_move(list);
    }
    SECTION("multiple insert") {
        Static_allocator_storage<1024> a;
        Static_allocator_storage<100>  b;
        Static_allocator_storage<1337> c;
        Unordered_memory_list          list(4);

        check_list(list, &a, 1024);
        check_list(list, &b, 100);
        check_list(list, &c, 1337);

        check_move(list);
    }
}

TEST_CASE("salt::detail::Memory_list", "[salt-memory/memory_list.hpp]") {
    SECTION("construct") {
        Memory_list list(4);
        REQUIRE(list.empty());
        REQUIRE(list.node_size() >= 4);
        REQUIRE(list.capacity() == 0u);
    }

    SECTION("normal insert") {
        Static_allocator_storage<1024> memory;
        Memory_list                    list(4);
        check_list(list, &memory, 1024);
        use_list_array(list);

        check_move(list);
    }
    SECTION("uneven insert") {
        Static_allocator_storage<1023> memory; // not dividable
        Memory_list                    list(4);
        check_list(list, &memory, 1023);
        use_list_array(list);

        check_move(list);
    }
    SECTION("multiple insert") {
        Static_allocator_storage<1024> a;
        Static_allocator_storage<100>  b;
        Static_allocator_storage<1337> c;
        Memory_list                    list(4);

        check_list(list, &a, 1024);
        use_list_array(list);
        check_list(list, &b, 100);
        use_list_array(list);
        check_list(list, &c, 1337);
        use_list_array(list);

        check_move(list);
    }
}
