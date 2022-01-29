#include <catch2/catch.hpp>

#include <salt/math/primitives.hpp>

TEST_CASE("salt::math::Point", "[salt/math/primitives.hpp]") {
    using namespace salt;

    Point p1{1, 1};
    REQUIRE(p1 == Point{1, 1});
    REQUIRE(p1 != Point{2, 2});

    // Addition
    {
        Point p2 = p1 + p1;
        REQUIRE(p2 == Point{2, 2});
        REQUIRE(p2 == p1 + 1);
        REQUIRE(p2 == 1 + p1);
    }
    // Subtracting
    {
        Point p2 = p1 - p1;
        REQUIRE(p2 == Point{0, 0});
        REQUIRE(p2 == p1 - 1);
        REQUIRE(p2 == 1 - p1);
    }
    // Multiplication
    {
        Point p2 = p1 * Point{2, 2};
        REQUIRE(p2 == Point{2, 2});
        REQUIRE(p2 == p1 * 2);
        REQUIRE(p2 == 2 * p1);
    }
    // Division
    {
        Point p2 = Point{4, 4} / Point{2, 2};
        REQUIRE(p2 == Point{2, 2});
        REQUIRE(p2 == Point{4, 4} / 2);
        REQUIRE(p2 == 4 / Point{2, 2});
    }
}

TEST_CASE("salt::math::Size", "[salt/math/primitives.hpp]") {
    using namespace salt;

    Size s1{640, 640};
    REQUIRE(s1 == Size{640, 640});
    REQUIRE(s1 != Size{641, 641});

    // Addition
    {
        Size s2 = s1 + s1;
        REQUIRE(s2 == Size{1280, 1280});
        REQUIRE(s2 == s1 + 640);
        REQUIRE(s2 == 640 + s1);
    }
    // Subtracting
    {
        Size s2 = s1 - s1;
        REQUIRE(s2 == Size{0, 0});
        REQUIRE(s2 == s1 - 640);
        REQUIRE(s2 == 640 - s1);
    }
    // Multiplication
    {
        Size s2 = s1 * Size{2, 2};
        REQUIRE(s2 == Size{1280, 1280});
        REQUIRE(s2 == s1 * 2);
        REQUIRE(s2 == 2 * s1);
    }
    // Division
    {
        Size s2 = Size{1280, 1280} / Size{2, 2};
        REQUIRE(s2 == Size{640, 640});
        REQUIRE(s2 == Size{1280, 1280} / 2);
        REQUIRE(s2 == 1280 / Size{2, 2});
    }
}