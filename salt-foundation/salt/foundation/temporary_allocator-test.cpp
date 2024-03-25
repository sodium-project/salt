#include <salt/foundation.hpp>

#include <catch2/catch.hpp>

using namespace salt;

template <typename T>
using stack_vector = std::vector<T, fdn::std_allocator_adapter<T, fdn::temporary_allocator>>;

TEST_CASE("salt::fdn::temporary_allocator", "[salt-foundation/temporary_allocator.hpp]") {

    SECTION("test temporary allocator") {
        fdn::temporary_allocator allocator;

        auto ptr = allocator.allocate(sizeof(char), alignof(char));
        INFO("ptr = " << ptr << ", max_alignment = " << fdn::detail::max_alignment);
        CHECK(fdn::is_aligned(ptr, alignof(char)));
        CHECK_FALSE(fdn::is_aligned(ptr, fdn::detail::max_alignment));
    }

    SECTION("test temporary vector") {
        fdn::temporary_allocator allocator;

        stack_vector<int> v{allocator};
        v.push_back(2023);
        CHECK(2023 == v[0]);
    }
}