#include <catch2/catch.hpp>
#include <salt/memory/detail/constexpr_memcpy.hpp>

constexpr unsigned char Banand[] = "Banand";
constexpr unsigned char Banane[] = "Banane";
constexpr unsigned char Bananf[] = "Bananf";

constexpr bool true1[]  = {true, true, true};
constexpr bool true2[]  = {true, true, true};
constexpr bool false1[] = {false, false, false};

using salt::memory::detail::constexpr_memcmp;
using salt::memory::detail::constexpr_memcmp_equal;
using salt::memory::detail::constexpr_memcpy;

constexpr bool char_copy() noexcept {
    unsigned char dest[4];

    constexpr_memcpy(dest, Banand + 1, sizeof dest);
    unsigned char expected[] = "anan";
    return constexpr_memcmp_equal(expected, dest, 4);
}

TEST_CASE("salt::memory::detail::constexpr_memcpy", "[salt-memory/constexpr_memcpy.hpp]") {
    SECTION("compile time") {
        STATIC_REQUIRE(char_copy());
    }

    SECTION("runtime") {
        unsigned char source[] = "once upon a midnight dreary...";
        unsigned char dest[4];

        constexpr_memcpy(dest, source, sizeof dest);
        unsigned char expected[] = "once";
        CHECK(constexpr_memcmp_equal(expected, dest, 4));
    }
}

TEST_CASE("salt::memory::detail::constexpr_memcmp", "[salt-memory/constexpr_memcpy.hpp]") {
    STATIC_REQUIRE(constexpr_memcmp(Banane, Banand, 6) == 1);
    STATIC_REQUIRE(constexpr_memcmp(Banane, Banane, 6) == 0);
    STATIC_REQUIRE(constexpr_memcmp(Banane, Bananf, 6) == -1);
}

TEST_CASE("salt::memory::detail::constexpr_memcmp_equal", "[salt-memory/constexpr_memcpy.hpp]") {
    STATIC_REQUIRE_FALSE(constexpr_memcmp_equal(Banane, Banand, 6));
    STATIC_REQUIRE(constexpr_memcmp_equal(Banane, Banane, 6));

    STATIC_REQUIRE_FALSE(constexpr_memcmp_equal(true1, false1, 3));
    STATIC_REQUIRE(constexpr_memcmp_equal(true1, true2, 3));
}