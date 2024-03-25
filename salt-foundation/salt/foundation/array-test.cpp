
#include <salt/foundation.hpp>

#include <catch2/catch.hpp>

struct empty {};

struct trivial_copy {
    int   i;
    float f;
};

struct nontrivial_copy {
    nontrivial_copy(nontrivial_copy const&) {}
    nontrivial_copy& operator=(nontrivial_copy const&) {
        return *this;
    }
};

#if __has_cpp_attribute(no_unique_address)
struct test_zero_size_array {
    using empty_array = salt::fdn::array<int, 0>;
    [[no_unique_address]] int         i;
    [[no_unique_address]] empty_array a;
};
static_assert(sizeof(test_zero_size_array) == sizeof(int));
#endif

// clang-format off
template <typename T>
constexpr void check_trivially_copyable() noexcept {
    using namespace salt;
    STATIC_REQUIRE(meta::trivially_copyable<fdn::array<T, 0>>);
    STATIC_REQUIRE(meta::trivially_copyable<fdn::array<T, 1>>);
    STATIC_REQUIRE(meta::trivially_copyable<fdn::array<T, 2>>);
    STATIC_REQUIRE(meta::trivially_copyable<fdn::array<T, 3>>);
}
// clang-format on

consteval bool uninitialized_relocate_array() noexcept {
    using namespace salt;
    // clang-format off
    struct dummy {
        bool constructed = false;
        constexpr dummy() noexcept = default;
        constexpr dummy(dummy&&) noexcept { constructed = true; }
    };
    // clang-format on
    auto const N      = 5u;
    using storage     = fdn::uninitialized_storage<dummy>;
    using source      = fdn::array<storage, N>;
    using destination = fdn::array<storage, N>;

    struct [[nodiscard]] adapter final {
        constexpr dummy& operator()(storage& value) const noexcept {
            return get<dummy&>(value);
        }
        constexpr dummy const& operator()(storage const& value) const noexcept {
            return get<dummy const&>(value);
        }
    };
    using It = fdn::iterator_adapter<typename source::iterator, adapter>;

    source      src;
    destination dest;

    auto src_begin = It{src.begin(), adapter{}};
    uninitialized_value_construct_n(src_begin, N);
    auto dest_begin = It{dest.begin(), adapter{}};
    uninitialized_value_construct_n(dest_begin, N);

    auto result = uninitialized_relocate(src_begin, src_begin + 3, dest_begin);
    (void)result;
    // clang-format off
    bool const constructed = get<dummy&>(dest[0]).constructed &&
                             get<dummy&>(dest[1]).constructed &&
                             get<dummy&>(dest[2]).constructed;
    // clang-format on
    fdn::destroy(dest_begin, dest_begin + 3);
    fdn::destroy(src_begin + 3, src_begin + N);
    return constructed;
}

TEST_CASE("salt::fdn::array", "[salt-foundation/array.hpp]") {

    SECTION("relocate array") {
        STATIC_REQUIRE(uninitialized_relocate_array());
    }

    SECTION("trivially copyable") {
        check_trivially_copyable<int>();
        check_trivially_copyable<double>();
        check_trivially_copyable<empty>();
        check_trivially_copyable<trivial_copy>();
    }

    SECTION("only array of zero size is trivially copyable when T is not") {
        using namespace salt;
        STATIC_REQUIRE(meta::trivially_copyable<fdn::array<nontrivial_copy, 0>>);
        STATIC_REQUIRE_FALSE(meta::trivially_copyable<fdn::array<nontrivial_copy, 1>>);
        STATIC_REQUIRE_FALSE(meta::trivially_copyable<fdn::array<nontrivial_copy, 2>>);
        STATIC_REQUIRE_FALSE(meta::trivially_copyable<fdn::array<nontrivial_copy, 3>>);
    }

    SECTION("basic functionality at compile time") {
        using namespace salt;
        constexpr fdn::array a = {1, 2, 3};
        STATIC_REQUIRE_FALSE(a.empty());
        STATIC_REQUIRE(a.size() == 3);
        STATIC_REQUIRE(a.max_size() == 3);

        STATIC_REQUIRE(a[0] == 1);
        STATIC_REQUIRE(a[1] == 2);
        STATIC_REQUIRE(a[2] == 3);

        STATIC_REQUIRE(a.front() == 1);
        STATIC_REQUIRE(a.back() == 3);
    }

    SECTION("basic functionality at run time") {
        using namespace salt;
        fdn::array a = {1, 2, 3};
        CHECK_FALSE(a.empty());
        CHECK(a.size() == 3);
        CHECK(a.max_size() == 3);

        CHECK(a[0] == 1);
        CHECK(a[1] == 2);
        CHECK(a[2] == 3);

        CHECK(a.front() == 1);
        CHECK(a.back() == 3);

        auto it = a.begin();
        CHECK(it != a.end());

        int b[3];
        for (auto i = 0; it != a.end(); ++it, ++i) {
            b[i] = *it;
        }
        CHECK(b[0] == 1);
        CHECK(b[1] == 2);
        CHECK(b[2] == 3);

        a[2]   = 30;
        int* c = a.data();
        CHECK(c[0] == 1);
        CHECK(c[1] == 2);
        CHECK(c[2] == 30);
    }

    SECTION("compare") {
        using namespace salt;
        fdn::array const a = {1, 2, 3};
        fdn::array const b = {1, 2, 3};
        fdn::array const c = {4, 5, 6};

        CHECK_FALSE(a.empty());
        CHECK(a.size() == 3);
        CHECK(a.max_size() == 3);

        CHECK(a[0] == b[0]);
        CHECK(a[1] == b[1]);
        CHECK(a[2] == b[2]);

        CHECK(a == b);
        CHECK(b == a);
        CHECK(c != a);
        CHECK(b != c);

        CHECK(a.front() == b.front());
        CHECK(c.front() != b.front());
        CHECK(a.back() == b.back());
        CHECK(c.back() != b.back());

        CHECK(a.cbegin() != a.cend());

        CHECK(*a.begin() == *b.begin());
        CHECK(*c.begin() != *a.begin());
        CHECK(*(a.end() - 1) == *(b.end() - 1));
        CHECK(*(c.end() - 1) != *(a.end() - 1));

        int const* pa = a.data();
        int const* pb = b.data();
        int const* pc = c.data();
        for (auto const& value : a) {
            CHECK(value == *pa);
            CHECK(value == *pb);
            CHECK(value != *pc);
            ++pa;
            ++pb;
            ++pc;
        }
    }

    SECTION("lexicographical compare") {
        {
            using namespace salt;
            constexpr fdn::array a = {'a', 'b', 'c'};
            constexpr fdn::array b = {'a', 'b', 'c'};
            constexpr fdn::array c = {'A', 'B', 'C'};

            STATIC_REQUIRE(a == b);
            STATIC_REQUIRE(b == a);
            STATIC_REQUIRE(c != a);
            STATIC_REQUIRE(b != c);

            STATIC_REQUIRE(a > c);
            STATIC_REQUIRE(c < b);
            STATIC_REQUIRE(a >= c);
            STATIC_REQUIRE(c <= b);

            STATIC_REQUIRE(meta::same_as<decltype(a <=> b), std::strong_ordering>);
        }
        {
            using namespace salt;
            fdn::array a = {'a', 'b', 'c'};
            fdn::array b = {'a', 'b', 'c'};
            fdn::array c = {'A', 'B', 'C'};
            CHECK(a == b);
            CHECK(b == a);
            CHECK(c != a);
            CHECK(b != c);

            CHECK(a > c);
            CHECK(c < b);
            CHECK(a >= c);
            CHECK(c <= b);
        }
    }
}