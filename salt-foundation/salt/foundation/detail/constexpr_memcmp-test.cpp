#include <catch2/catch.hpp>
#include <salt/foundation/detail/constexpr_memcmp.hpp>

constexpr unsigned char Banand[] = "Banand";
constexpr unsigned char Banane[] = "Banane";
constexpr unsigned char Bananf[] = "Bananf";

constexpr bool true1[]  = {true, true, true};
constexpr bool true2[]  = {true, true, true};
constexpr bool false1[] = {false, false, false};

using salt::fdn::detail::constexpr_memcmp;
using salt::fdn::detail::constexpr_memcmp_equal;

TEST_CASE("salt::fdn::detail::constexpr_memcmp", "[salt-foundation/constexpr_memcmp.hpp]") {
    SECTION("constant evaluated") {
        STATIC_REQUIRE(constexpr_memcmp(Banane, Banand, 6) == 1);
        STATIC_REQUIRE(constexpr_memcmp(Banane, Banane, 6) == 0);
        STATIC_REQUIRE(constexpr_memcmp(Banane, Bananf, 6) == -1);
    }

    SECTION("basic") {
        CHECK(constexpr_memcmp(Banane, Banand, 6) == 1);
        CHECK(constexpr_memcmp(Banane, Banane, 6) == 0);
        CHECK(constexpr_memcmp(Banane, Bananf, 6) == -1);
    }
}

TEST_CASE("salt::fdn::detail::constexpr_memcmp_equal", "[salt-foundation/constexpr_memcmp_equal.hpp]") {
    SECTION("constant evaluated") {
        STATIC_REQUIRE_FALSE(constexpr_memcmp_equal(Banane, Banand, 6));
        STATIC_REQUIRE(constexpr_memcmp_equal(Banane, Banane, 6));

        STATIC_REQUIRE_FALSE(constexpr_memcmp_equal(true1, false1, 3));
        STATIC_REQUIRE(constexpr_memcmp_equal(true1, true2, 3));
    }

    SECTION("basic") {
        CHECK_FALSE(constexpr_memcmp_equal(Banane, Banand, 6));
        CHECK(constexpr_memcmp_equal(Banane, Banane, 6));

        CHECK_FALSE(constexpr_memcmp_equal(true1, false1, 3));
        CHECK(constexpr_memcmp_equal(true1, true2, 3));
    }
}