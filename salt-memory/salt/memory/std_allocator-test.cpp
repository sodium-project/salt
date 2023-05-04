#include <catch2/catch.hpp>

#include <salt/memory/default_allocator.hpp>
#include <salt/memory/detail/test_allocator.hpp>
#include <salt/memory/std_allocator.hpp>

namespace unit_tests {

struct [[nodiscard]] Typeless_mallocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    Typeless_mallocator() = default;

    [[nodiscard]] void* allocate(size_type n /* bytes */) {
        return std::malloc(n * sizeof(std::byte));
    }

    void deallocate(void* p, size_type) noexcept {
        std::free(p);
    }
};

} // namespace unit_tests

using std_stateful_allocator = salt::Std_allocator<int, Test_allocator>;
static_assert(std::same_as<std_stateful_allocator::propagate_on_container_copy_assignment, std::false_type>);
static_assert(std::same_as<std_stateful_allocator::propagate_on_container_move_assignment, std::false_type>);
static_assert(std::same_as<std_stateful_allocator::propagate_on_container_swap, std::false_type>);

using std_stateless_allocator = salt::Std_allocator<int, std::allocator<int>>;
static_assert(std::same_as<std_stateless_allocator::propagate_on_container_copy_assignment, std::true_type>);
static_assert(std::same_as<std_stateless_allocator::propagate_on_container_move_assignment, std::true_type>);
static_assert(std::same_as<std_stateless_allocator::propagate_on_container_swap, std::false_type>);

using std_any_allocator = salt::Std_any_allocator<int>;
static_assert(std::same_as<std_any_allocator::propagate_on_container_copy_assignment, std::false_type>);
static_assert(std::same_as<std_any_allocator::propagate_on_container_move_assignment, std::false_type>);
static_assert(std::same_as<std_any_allocator::propagate_on_container_swap, std::false_type>);

TEST_CASE("salt::Std_allocator", "[salt-memory/std_allocator.hpp]") {
    using namespace salt;
    using namespace salt::detail;

    SECTION("test std_stateful_allocator") {
        Test_allocator         allocator;
        std_stateful_allocator adapter{allocator};

        auto* ptr = adapter.allocate(sizeof(int));
        REQUIRE(is_aligned(ptr, alignof(int)));
        REQUIRE(allocator.no_allocated() == 1u);
        REQUIRE(allocator.no_deallocated() == 0u);

        adapter.deallocate(ptr, sizeof(int));
        REQUIRE(allocator.no_allocated() == 0u);
        REQUIRE(allocator.no_deallocated() == 1u);
    }

    SECTION("test std_stateless_allocator") {
        std_stateless_allocator adapter;

        auto* ptr = adapter.allocate(sizeof(int));
        REQUIRE(is_aligned(ptr, alignof(int)));
        adapter.deallocate(ptr, sizeof(int));
    }

    SECTION("test std_any_allocator") {
        std_any_allocator adapter{std::allocator<int>{}};

        auto* ptr = adapter.allocate(sizeof(int));
        REQUIRE(is_aligned(ptr, alignof(int)));
        adapter.deallocate(ptr, sizeof(int));
    }

    SECTION("test std_any_allocator with Typeless_mallocator") {
        std_any_allocator adapter{unit_tests::Typeless_mallocator{}};

        auto* ptr = adapter.allocate(sizeof(int));
        REQUIRE(ptr);
        REQUIRE(is_aligned(ptr, alignof(int)));
        adapter.deallocate(ptr, sizeof(int));
    }
}