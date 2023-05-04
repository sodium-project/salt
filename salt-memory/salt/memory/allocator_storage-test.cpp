#include <catch2/catch.hpp>

#include <salt/memory/allocator_storage.hpp>
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/test_allocator.hpp>

template <typename RawAllocator>
void do_allocate_node(salt::Allocator_reference<RawAllocator> reference) {
    using namespace salt::detail;
    auto* node = reference.allocate_node(sizeof(int), alignof(int));
    REQUIRE(node);
    REQUIRE(is_aligned(node, 4u));
    reference.deallocate_node(node, sizeof(int), alignof(int));
}

static_assert(salt::is_raw_allocator<std::allocator<int>>);

TEST_CASE("salt::Allocator_storage", "[salt-memory/allocator_storage.hpp]") {
    using namespace salt;

    SECTION("test is_composable()") {
        Allocator_reference<Test_allocator> ref;
        REQUIRE_FALSE(ref.is_composable());
    }

    SECTION("test stateful reference") {
        Test_allocator                      allocator;
        Allocator_reference<Test_allocator> ref_stateful(allocator);
        do_allocate_node(ref_stateful);
    }

    SECTION("test stateless reference") {
        Allocator_reference<Heap_allocator> ref_stateless(Heap_allocator{});
        do_allocate_node(ref_stateless);
    }

    SECTION("test any allocator reference") {
        Any_allocator_reference any_ref{std::allocator<int>{}};
        REQUIRE_FALSE(any_ref.is_composable());
        do_allocate_node(any_ref);
    }
}