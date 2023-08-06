#include <catch2/catch.hpp>
#include <salt/foundation/algorithm.hpp>

using salt::fdn::equal;

template <typename UnderlyingType> constexpr bool test_equal() noexcept {
    UnderlyingType a[]  = {0, 1, 2, 3, 4, 5};
    const unsigned s    = sizeof(a) / sizeof(a[0]);
    UnderlyingType b[s] = {0, 1, 2, 5, 4, 5};

    return equal(a, a + s, a) && !equal(a, a + s, b);
}

class dummy {
public:
    constexpr dummy(int i) noexcept : i_(i) {}

    constexpr bool operator==(const dummy&) const noexcept = default;

private:
    int i_;
};

struct base {};
struct derived : virtual base {};

TEST_CASE("salt::fdn::equal", "[salt-foundation/algorithm.hpp]") {
    SECTION("fundamental types") {
        CHECK(test_equal<const char>());
        CHECK(test_equal<salt::fdn::u8>());
        CHECK(test_equal<const int>());
        CHECK(test_equal<salt::fdn::u64>());
        CHECK(test_equal<float>());
    }

    SECTION("user-defined equality comparable types") {
        STATIC_REQUIRE(test_equal<dummy>());
    }

    SECTION("user-defined types with virtual base") {
        derived  d;
        derived* a[] = {&d, nullptr};
        base*    b[] = {&d, nullptr};
        CHECK(equal(a, a + 2, b));
    }
}