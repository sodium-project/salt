#include <salt/algorithm/equal.hpp>
#include <salt/utility/types.hpp>

#include <catch2/catch.hpp>

// clang-format off
template <typename UnderlyingType>
constexpr bool test_equal() noexcept {
    using namespace salt::algorithm;
    UnderlyingType a[]  = {0, 1, 2, 3, 4, 5};
    const unsigned s    = sizeof(a) / sizeof(a[0]);
    UnderlyingType b[s] = {0, 1, 2, 5, 4, 5};

    return equal(a, a + s, a) && !equal(a, a + s, b);
}
// clang-format on

class dummy {
public:
    constexpr dummy(int i) noexcept : i_(i) {}

    constexpr bool operator==(const dummy&) const noexcept = default;

private:
    int i_;
};

struct base {};
struct derived : virtual base {};

TEST_CASE("salt::algorithm::equal", "[salt-algorithm/equal.hpp]") {
    SECTION("fundamental types") {
        CHECK(test_equal<const char>());
        CHECK(test_equal<salt::ts::u8>());
        CHECK(test_equal<const int>());
        CHECK(test_equal<salt::ts::u64>());
        CHECK(test_equal<float>());
    }

    SECTION("user-defined equality comparable types") {
        STATIC_REQUIRE(test_equal<dummy>());
        CHECK(test_equal<dummy>());
    }

    SECTION("user-defined types with virtual base") {
        derived  d;
        derived* a[] = {&d, nullptr};
        base*    b[] = {&d, nullptr};
        CHECK(salt::algorithm::equal(a, a + 2, b));
    }
}
