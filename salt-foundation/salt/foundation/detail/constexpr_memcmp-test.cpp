#include <catch2/catch.hpp>
#include <salt/foundation/detail/constexpr_memcmp.hpp>

constexpr unsigned char Banand[] = "Banand";
constexpr unsigned char Banane[] = "Banane";
constexpr unsigned char Bananf[] = "Bananf";

using salt::fdn::detail::constexpr_memcmp;
using salt::fdn::detail::constexpr_memcmp_equal;

TEST_CASE("salt::fdn::detail::constexpr_memcmp", "[salt-foundation/constexpr_memcmp.hpp]") {
    STATIC_REQUIRE(constexpr_memcmp(Banane, Banand, 6) == 1);
    STATIC_REQUIRE(constexpr_memcmp(Banane, Banane, 6) == 0);
    STATIC_REQUIRE(constexpr_memcmp(Banane, Bananf, 6) == -1);
}

TEST_CASE("salt::fdn::detail::constexpr_memcmp_equal", "[salt-foundation/constexpr_memcmp_equal.hpp]") {
    STATIC_REQUIRE_FALSE(constexpr_memcmp_equal(Banane, Banand, 6));
    STATIC_REQUIRE(constexpr_memcmp_equal(Banane, Banane, 6));
}