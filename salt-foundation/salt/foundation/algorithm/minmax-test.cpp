#include <salt/foundation/algorithm/minmax.hpp>

#include <catch2/catch.hpp>

TEST_CASE("salt::algorithm::min", "[salt-algorithm/minmax.hpp]") {
    using namespace salt;
    CHECK(algorithm::min(1, 2) == 1);
    CHECK(algorithm::min(2, 1) == 1);
    CHECK(algorithm::min(1, 1) == 1);
}

TEST_CASE("salt::algorithm::max", "[salt-algorithm/minmax.hpp]") {
    using namespace salt;
    CHECK(algorithm::max(1, 2) == 2);
    CHECK(algorithm::max(2, 1) == 2);
    CHECK(algorithm::max(1, 1) == 1);
}
