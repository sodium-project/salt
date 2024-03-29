#include <catch2/catch.hpp>

#include <salt/foundation.hpp>

TEST_CASE("salt::Static_storage", "[salt-foundation/static_storage.hpp]") {
    using namespace salt;

    SECTION("it must be given valid Size and Alignment") {
        REQUIRE_FALSE(not requires {
            // This code won't compile.
            Static_storage<int, 1, 1>{std::in_place, 2022};
        });

        REQUIRE(requires {
            // This code will compile.
            Static_storage<int, sizeof(int), alignof(int)>{std::in_place, 2022};
        });
    }

    SECTION("it can be constructed with arguments") {
        Static_storage<int, 4, 4> a{std::in_place, 36};
        REQUIRE(*a == 36);
    }

    SECTION("it is copy constructible") {
        Static_storage<int, 4, 4> a{std::in_place, 36};
        Static_storage            b{a};
        REQUIRE(*b == 36);
    }

    SECTION("it is move constructible") {
        Static_storage_for<std::string> a{std::in_place, "hello"};
        Static_storage                  b{std::move(a)};
        REQUIRE(*b == "hello");
    }

    SECTION("it is copy assignable") {
        Static_storage<int, 4, 4> a{std::in_place, 36};
        Static_storage<int, 4, 4> b{std::in_place, 12};
        a = b;
        REQUIRE(*a == 12);
    }

    SECTION("it is move assignable") {
        Static_storage_for<std::string> a{std::in_place, "hello"};
        Static_storage_for<std::string> b{std::in_place, "world"};
        a = std::move(b);
        REQUIRE(*a == "world");
    }

    SECTION("it is equality comparable") {
        Static_storage_for<std::string> a{std::in_place, "hello"};
        Static_storage_for<std::string> b{std::in_place, "world"};
        Static_storage_for<std::string> c{std::in_place, "hello"};
        REQUIRE(a != b);
        REQUIRE(a == c);
        REQUIRE(b != c);
    }
}