#include <catch2/catch.hpp>

#include <salt/memory/containers.hpp>
#include <salt/memory/memory_pool.hpp>
#include <salt/memory/smart_ptr.hpp>

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

template <typename Allocator>
using shared_ptr_size = salt::memory::allocate_shared_node_size<int, Allocator>;

TEST_CASE("salt::allocate_shared", "[salt-memory/smart_ptr.hpp]") {
    using namespace salt;

    SECTION("stateless") {
        Dummy_allocator::size = 0;
        auto ptr              = allocate_shared<int>(Dummy_allocator{}, 42);
        REQUIRE(*ptr == 42);
        REQUIRE((Dummy_allocator::size <= shared_ptr_size<Dummy_allocator>::value));
    }

    SECTION("stateful") {
        using memory_pool = Memory_pool<>;
        memory_pool pool(shared_ptr_size<memory_pool>::value, 1024);
        auto        ptr = allocate_shared<int>(pool, 42);
        REQUIRE(*ptr == 42);
        REQUIRE((Dummy_allocator::size <= shared_ptr_size<Dummy_allocator>::value));
    }
}

TEST_CASE("salt::allocate_unique", "[salt-memory/smart_ptr.hpp]") {
    using namespace salt;

    SECTION("stateless") {
        Dummy_allocator::size = 0;
        auto ptr              = allocate_unique<int>(Dummy_allocator{}, 42);
        REQUIRE(*ptr == 42);
        REQUIRE(Dummy_allocator::size == sizeof(int));
    }

    SECTION("stateful") {
        using memory_pool = Memory_pool<>;
        memory_pool pool(sizeof(int), 1024);
        auto        ptr = allocate_shared<int>(pool, 42);
        REQUIRE(*ptr == 42);
        REQUIRE(Dummy_allocator::size == sizeof(int));
    }
}
