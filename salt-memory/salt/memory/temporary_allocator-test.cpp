#include <catch2/catch.hpp>

#include <salt/memory/temporary_allocator.hpp>

TEST_CASE("salt::Temporary_allocator", "[salt-memory/temporary_allocator.hpp]") {
    using namespace salt;
    using namespace salt::detail;

    Temporary_allocator allocator;

    auto ptr = allocator.allocate(sizeof(char), alignof(char));
    REQUIRE(is_aligned(ptr, alignof(char)));
    REQUIRE_FALSE(is_aligned(ptr, max_alignment));
}