#include <catch2/catch.hpp>

#include <salt/math/compare.hpp>

TEST_CASE("compare doubles", "[salt/math/compare.hpp]") {
    using namespace salt;

    REQUIRE(compare(0.001, 0.002, 0.001) == std::weak_ordering::equivalent);
    REQUIRE(compare(0.001, 0.003, 0.001) == std::weak_ordering::less);
    REQUIRE(compare(0.011, 0.003, 0.001) == std::weak_ordering::greater);
}

TEST_CASE("compare floats", "[salt/math/compare.hpp]") {
    using namespace salt;

    REQUIRE(compare(0.001f, 0.002f, 0.001f) == std::weak_ordering::equivalent);
    REQUIRE(compare(0.001f, 0.003f, 0.001f) == std::weak_ordering::less);
    REQUIRE(compare(0.011f, 0.003f, 0.001f) == std::weak_ordering::greater);
}

TEST_CASE("compare ints", "[salt/math/compare.hpp]") {
    using namespace salt;

    REQUIRE(compare(0, 0) == std::strong_ordering::equal);
    REQUIRE(compare(0, 1) == std::strong_ordering::less);
    REQUIRE(compare(2, 1) == std::strong_ordering::greater);
}

TEST_CASE("Comparing vectors", "[salt/math/compare.hpp]") {
    using namespace salt;

    REQUIRE(compare(glm::dvec4{1, 2, 3, 4}, glm::dvec3{1, 2, 3}, 1e-3) == std::weak_ordering::equivalent);
    REQUIRE(compare(glm::dvec4{1, 2, 3, 4}, glm::dvec3{1, 2, 4}, 1e-3) == std::weak_ordering::less);
    REQUIRE(compare(glm::dvec4{1, 3, 3, 4}, glm::dvec3{1, 2, 4}, 1e-3) == std::weak_ordering::less);
    REQUIRE(compare(glm::dvec4{1, 3, 3, 4}, glm::dvec3{1, 2, 3}, 1e-3) == std::weak_ordering::greater);
}