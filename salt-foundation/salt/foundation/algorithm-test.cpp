#include <catch2/catch.hpp>
#include <salt/foundation/algorithm.hpp>
#include <salt/foundation/types.hpp>

using salt::fdn::equal;

// clang-format off
template <typename UnderlyingType>
constexpr bool test_equal() noexcept {
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

TEST_CASE("salt::fdn::min", "[salt-foundation/algorithm.hpp]") {
    using namespace salt;
    CHECK(fdn::min(1, 2) == 1);
    CHECK(fdn::min(2, 1) == 1);
    CHECK(fdn::min(1, 1) == 1);
}

TEST_CASE("salt::fdn::max", "[salt-foundation/algorithm.hpp]") {
    using namespace salt;
    CHECK(fdn::max(1, 2) == 2);
    CHECK(fdn::max(2, 1) == 2);
    CHECK(fdn::max(1, 1) == 1);
}

template <class Int1, class Int2, class LexCmpThreeWayFn, class Cmp = std::compare_three_way>
void test_three_way_for_integrals(LexCmpThreeWayFn lex_cmp_three_way_fn, Cmp cmp = {}) {
    Int1 arr1[10] = {5, 7, 3, 4, 6, 4, 7, 1, 9, 5};
    Int2 arr2[10] = {5, 7, 3, 4, 6, 4, 7, 1, 9, 5};
    Int2 arr3[10] = {5, 7, 3, 4, 6, 4, 7, 1, 10, 5};
    Int2 arr4[10] = {5, 7, 2, 4, 6, 4, 7, 1, 9, 5};
    Int2 arr5[11] = {5, 7, 3, 4, 6, 4, 7, 1, 9, 5, 4};
    Int2 arr6[11] = {5, 7, 3, 4, 6, 4, 7, 1, 10, 5, 4};
    Int2 arr7[11] = {5, 7, 2, 4, 6, 4, 7, 1, 9, 5, 4};
    Int2 arr8[9]  = {5, 7, 3, 4, 6, 4, 7, 1, 9};
    Int2 arr9[9]  = {5, 7, 3, 4, 6, 4, 7, 1, 10};
    Int2 arr10[9] = {5, 7, 2, 4, 6, 4, 7, 1, 9};

    using ordering = decltype(cmp(arr1[0], arr2[0]));
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr2, arr2 + 10, cmp) == ordering::equivalent);
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr3, arr3 + 10, cmp) == ordering::less);
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr4, arr4 + 10, cmp) == ordering::greater);

    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr5, arr5 + 11, cmp) == ordering::less);
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr6, arr6 + 11, cmp) == ordering::less);
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr7, arr7 + 11, cmp) == ordering::greater);

    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr8, arr8 + 9, cmp) == ordering::greater);
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr9, arr9 + 9, cmp) == ordering::less);
    CHECK(lex_cmp_three_way_fn(arr1, arr1 + 10, arr10, arr10 + 9, cmp) == ordering::greater);
}

TEST_CASE("salt::fdn::lexicographical_compare_three_way", "[salt-foundation/algorithm.hpp]") {
    using namespace salt;

    const auto lex_cmp_three_way_fn = [](auto begin1, auto end1, auto begin2, auto end2, auto cmp) {
        return fdn::lexicographical_compare_three_way(begin1, end1, begin2, end2, cmp);
    };
    const auto lex_cmp_three_way_slow_fn = [](auto begin1, auto end1, auto begin2, auto end2,
                                              auto cmp) {
        return fdn::detail::lexicographical_compare_three_way_slow(begin1, end1, begin2, end2, cmp);
    };

    SECTION("integers") {
        test_three_way_for_integrals<int, int>(lex_cmp_three_way_fn, std::compare_three_way{});
        test_three_way_for_integrals<int, int>(lex_cmp_three_way_fn, std::strong_order);
        test_three_way_for_integrals<int, int>(lex_cmp_three_way_fn, std::weak_order);
        test_three_way_for_integrals<int, int>(lex_cmp_three_way_fn, std::partial_order);
        test_three_way_for_integrals<int, int>(lex_cmp_three_way_slow_fn, std::compare_three_way{});

        using ordering = std::strong_ordering;
        using cmp      = std::compare_three_way;
        int arr1[]     = {1};
        int arr2[]     = {2};
        int arr3[]     = {3, 4, 5};
        CHECK(lex_cmp_three_way_fn(arr1, arr1, arr3, arr3 + 3, cmp{}) == ordering::less);
        CHECK(lex_cmp_three_way_fn(arr3, arr3 + 3, arr1, arr1, cmp{}) == ordering::greater);
        CHECK(lex_cmp_three_way_fn(arr1, arr1, arr2, arr2, cmp{}) == ordering::equal);
    }

    SECTION("chars") {
        test_three_way_for_integrals<char, char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<signed char, signed char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<char, signed char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<signed char, char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<unsigned char, unsigned char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<unsigned char, char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<char, unsigned char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<unsigned char, signed char>(lex_cmp_three_way_fn);
        test_three_way_for_integrals<signed char, unsigned char>(lex_cmp_three_way_fn);
    }
}