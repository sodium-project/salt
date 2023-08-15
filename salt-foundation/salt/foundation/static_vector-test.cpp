#include <catch2/catch.hpp>
#include <salt/foundation/static_vector.hpp>

namespace trivially_copyable_vector {

using vector_type = salt::fdn::static_vector<int, 5>;

static_assert(not salt::meta::trivial<vector_type>);
static_assert(salt::meta::trivially_copyable<vector_type>);
static_assert(salt::meta::standard_layout<vector_type>);

} // namespace trivially_copyable_vector

struct copyable_or_movable {
    copyable_or_movable() = default;

    constexpr copyable_or_movable(copyable_or_movable const& other) noexcept = delete;
    constexpr copyable_or_movable(copyable_or_movable&& other) noexcept      = delete;

    constexpr copyable_or_movable& operator=(copyable_or_movable const& other) noexcept = delete;
    constexpr copyable_or_movable& operator=(copyable_or_movable&& other) noexcept      = delete;
};

namespace copyable_or_movable_vector {

using vector_type = salt::fdn::static_vector<copyable_or_movable, 5>;

static_assert(not salt::meta::trivially_copyable<vector_type>);
static_assert(salt::meta::copy_assignable<vector_type>);
static_assert(salt::meta::copy_constructible<vector_type>);
static_assert(salt::meta::move_assignable<vector_type>);
static_assert(salt::meta::trivially_destructible<vector_type>);

} // namespace copyable_or_movable_vector

struct nontrivial_int {
    int x = 0;

    constexpr nontrivial_int() : x{0} {}

    constexpr nontrivial_int(int val) : x{val} {}
    constexpr ~nontrivial_int() {}

    constexpr nontrivial_int(nontrivial_int const& other) noexcept {
        x = other.x;
    }
    constexpr nontrivial_int(nontrivial_int&& other) noexcept {
        x = other.x;
    }

    constexpr nontrivial_int& operator=(nontrivial_int const& other) noexcept {
        x = other.x;
        return *this;
    }
    constexpr nontrivial_int& operator=(nontrivial_int&& other) noexcept {
        x = other.x;
        return *this;
    }

    constexpr operator int() noexcept {
        return x;
    }

    constexpr bool operator==(nontrivial_int const& other) const {
        return x == other.x;
    }
};

namespace not_trivially_copyable_vector {

using vector_type = salt::fdn::static_vector<nontrivial_int, 5>;

static_assert(not salt::meta::trivially_copyable<vector_type>);
static_assert(not salt::meta::trivially_copy_constructible<vector_type>);
static_assert(not salt::meta::trivially_copy_assignable<vector_type>);
static_assert(not salt::meta::trivially_move_assignable<vector_type>);
static_assert(not salt::meta::trivially_destructible<vector_type>);

} // namespace not_trivially_copyable_vector

struct complex_type {
    constexpr complex_type(int param_a, int param_b1, int param_b2, int param_c)
            : a(param_a), b({param_b1, param_b2}), c(param_c) {}

    int                      a;
    salt::fdn::array<int, 2> b;
    int                      c;
};

TEST_CASE("salt::fdn::static_vector", "[salt-foundation/static_vector.hpp]") {
    using namespace salt;

    SECTION("default constructor") {
        {
            constexpr fdn::static_vector<int, 5> v{};
            STATIC_REQUIRE(v.empty());
            STATIC_REQUIRE(v.max_size() == 5);
            STATIC_REQUIRE(v.capacity() == 5);
        }
        {
            fdn::static_vector<int, 5> v{};
            CHECK(v.empty());
            CHECK(v.max_size() == 5);
            CHECK(v.capacity() == 5);
        }
    }

    SECTION("count constructor") {
        {
            fdn::static_vector<int, 5> v(5);
            CHECK_FALSE(v.empty());
            CHECK(v.max_size() == 5);
            CHECK(v.capacity() == 5);
            CHECK(v == fdn::static_vector{0, 0, 0, 0, 0});
        }
        {
            fdn::static_vector<int, 5> v(5, 3);
            CHECK_FALSE(v.empty());
            CHECK(v.max_size() == 5);
            CHECK(v.capacity() == 5);
            CHECK(v == fdn::static_vector{3, 3, 3, 3, 3});
        }
    }

    SECTION("list constructor") {
        fdn::static_vector<int, 5> v{1, 2, 3};
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
        CHECK(v.size() == 3);
        CHECK(v.max_size() == 5);
    }

    SECTION("range constructor") {
        {
            constexpr fdn::static_vector<int, 3> v1{7, 9};
            STATIC_REQUIRE(v1[0] == 7);
            STATIC_REQUIRE(v1[1] == 9);
            STATIC_REQUIRE(v1.size() == 2);
            constexpr fdn::static_vector<int, 5> v2{v1.begin(), v1.end()};
            STATIC_REQUIRE(v2[0] == 7);
            STATIC_REQUIRE(v2[1] == 9);
            STATIC_REQUIRE(v2.size() == 2);
        }
        {
            fdn::static_vector<int, 3> v1{2, 1};
            CHECK(v1[0] == 2);
            CHECK(v1[1] == 1);
            CHECK(v1.size() == 2);
            fdn::static_vector<int, 5> v2{v1.begin(), v1.end()};
            CHECK(v2[0] == 2);
            CHECK(v2[1] == 1);
            CHECK(v2.size() == 2);
        }
    }

    SECTION("copy constructor") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<nontrivial_int, 3> v;
                v.emplace_back(1);
                v.emplace_back(2);
                return v;
            }();
            constexpr fdn::static_vector<nontrivial_int, 3> v2{v1};
            STATIC_REQUIRE(v1 == v2);
        }
        {
            fdn::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fdn::static_vector<nontrivial_int, 3> v2{v1};
            CHECK(v1 == v2);
        }
    }

    SECTION("move constructor") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<nontrivial_int, 3> v;
                v.emplace_back(1);
                v.emplace_back(2);
                return v;
            }();
            constexpr fdn::static_vector<nontrivial_int, 3> v2{meta::move(v1)};
            STATIC_REQUIRE(v1 == v2);
        }
        {
            fdn::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fdn::static_vector<nontrivial_int, 3> v2{meta::move(v1)};
            CHECK(v1 == v2);
        }
        {
            fdn::static_vector<int, 5> v1{5, 6, 7};
            fdn::static_vector<int, 5> v2{meta::move(v1)};
            CHECK(v1 == v2);
        }
    }

    SECTION("copy assignment") {
        {
            constexpr auto cmp = []() {
                fdn::static_vector<nontrivial_int, 3> v1;
                v1.emplace_back(1);
                v1.emplace_back(2);
                fdn::static_vector<nontrivial_int, 3> v2{};
                v2 = v1;
                return v1 == v2;
            }();
            STATIC_REQUIRE(cmp);
        }
        {
            fdn::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fdn::static_vector<nontrivial_int, 3> v2;
            v2 = v1;
            CHECK(v1 == v2);
        }
    }

    SECTION("move assignment") {
        {
            constexpr auto cmp = []() {
                fdn::static_vector<nontrivial_int, 3> v1;
                v1.emplace_back(1);
                v1.emplace_back(2);
                fdn::static_vector<nontrivial_int, 3> v2{};
                v2 = meta::move(v1);
                return v1 == v2;
            }();
            STATIC_REQUIRE(cmp);
        }
        {
            fdn::static_vector<nontrivial_int, 3> v1;
            v1.emplace_back(1);
            v1.emplace_back(2);

            fdn::static_vector<nontrivial_int, 3> v2;
            v2 = meta::move(v1);
            CHECK(v1 == v2);
        }
        {
            fdn::static_vector<int, 5> v1{1, 2};
            fdn::static_vector<int, 5> v2;
            v2 = meta::move(v1);
            CHECK(v1 == v2);
        }
    }

    SECTION("equality operator") {
        constexpr fdn::static_vector vector{1, 2, 3};
        STATIC_REQUIRE(vector == fdn::static_vector{1, 2, 3});
        STATIC_REQUIRE(vector != fdn::static_vector{1, 2, 1});
    }

    SECTION("assign") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v;
                v.assign({1, 2, 3, 4, 5});
                return v;
            }();
            STATIC_REQUIRE_FALSE(v1.empty());
            STATIC_REQUIRE(v1 == fdn::static_vector{1, 2, 3, 4, 5});
        }
        {
            fdn::static_vector<int, 5> v;
            v.assign({1, 2, 3});
            CHECK_FALSE(v.empty());
            CHECK(v == fdn::static_vector<int, 5>{1, 2, 3});
            v.assign(2, 5);
            CHECK(v == fdn::static_vector<int, 5>{5, 5});
        }
        {
            fdn::static_vector<nontrivial_int, 3> v1{10, 20, 30};
            fdn::static_vector<nontrivial_int, 3> v2;
            v2.assign(v1.begin(), v1.end());
            CHECK(v1 == v2);
        }
    }

    SECTION("clear") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{1, 2, 3, 4, 5};
                v.clear();
                return v;
            }();
            STATIC_REQUIRE(v1.empty());
            STATIC_REQUIRE(v1.max_size() == 5);
            STATIC_REQUIRE(v1.capacity() == 5);
        }
        {
            fdn::static_vector<int, 5> v{1, 2, 3, 4, 5};
            v.clear();
            CHECK(v.empty());
            CHECK(v.max_size() == 5);
            CHECK(v.capacity() == 5);
        }
        {
            fdn::static_vector<nontrivial_int, 3> v{1, 2, 3};
            v.clear();
            CHECK(v.empty());
            CHECK(v.max_size() == 3);
            CHECK(v.capacity() == 3);
        }
    }

    SECTION("resize") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{1, 2, 3, 4, 5};
                v.resize(3);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 5>{1, 2, 3});
            STATIC_REQUIRE(v1.capacity() == 5);
        }
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{1, 2};
                v.resize(4, 7);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 5>{1, 2, 7, 7});
            STATIC_REQUIRE(v1.capacity() == 5);
        }
        {
            fdn::static_vector<int, 5> v{1, 2, 3, 4, 5};
            v.resize(3);
            CHECK(v == fdn::static_vector<int, 5>{1, 2, 3});
        }
        {
            fdn::static_vector<nontrivial_int, 5> v{1, 2};

            auto const x = nontrivial_int{7};
            v.resize(4, x);
            CHECK(v == fdn::static_vector<nontrivial_int, 5>{1, 2, 7, 7});
        }
    }

    SECTION("data") {
        fdn::static_vector<int, 3> v{1, 2, 3};

        int* p = v.data();
        CHECK(p[0] == 1);
        CHECK(p[1] == 2);
        CHECK(p[2] == 3);

        auto const& cref_v = v;
        int const*  cp     = cref_v.data();
        CHECK(cp[0] == 1);
        CHECK(cp[1] == 2);
        CHECK(cp[2] == 3);
    }

    SECTION("front/back") {
        fdn::static_vector<int, 3> v{1, 2, 3};
        CHECK(v.front() == 1);
        CHECK(v.back() == 3);

        auto const& cref_v = v;
        CHECK(cref_v.front() == 1);
        CHECK(cref_v.back() == 3);
    }

    SECTION("subsript operator") {
        fdn::static_vector<int, 3> const v{1, 2, 3};
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
        CHECK(v.cbegin() != v.cend());
    }

    SECTION("push_back") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{1, 2};
                v.push_back(5);
                int const value = 8;
                v.push_back(value);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 5>{1, 2, 5, 8});
        }
        {
            fdn::static_vector<int, 3> v;
            v.push_back(1);
            int const x = 2;
            v.push_back(x);
            v.push_back(3);
            CHECK_FALSE(v.empty());
            CHECK(v.size() == 3);
            CHECK(v.max_size() == 3);
            CHECK(v == fdn::static_vector{1, 2, 3});
        }
    }

    SECTION("emplace_back") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{1, 2};
                v.emplace_back(5);
                int const value = 8;
                v.emplace_back(value);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 5>{1, 2, 5, 8});
        }
        {
            fdn::static_vector<complex_type, 3> v;
            v.emplace_back(1, 2, 3, 4);
            auto ref = v.emplace_back(101, 202, 303, 404);
            CHECK(ref.a == 101);
            CHECK(ref.c == 404);
        }
    }

    SECTION("reserve") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 6> v{};
                v.reserve(5);
                return v;
            }();
            STATIC_REQUIRE(v1.max_size() == 6);
            STATIC_REQUIRE(v1.capacity() == 6);
        }
        {
            fdn::static_vector<int, 10> v;
            v.reserve(8);
            CHECK(v.max_size() == 10);
            CHECK(v.capacity() == 10);
        }
    }

    SECTION("pop_back") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 3> v{0, 1, 2};
                v.pop_back();
                return v;
            }();
            STATIC_REQUIRE(v1.size() == 2);
            STATIC_REQUIRE(v1.capacity() == 3);
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 3>{0, 1});
        }
        {
            fdn::static_vector<int, 4> v{5, 6, 7};
            v.pop_back();
            CHECK(v == fdn::static_vector<int, 4>{5, 6});
        }
    }

    SECTION("insert") {
        {
            fdn::static_vector<int, 8> v;

            auto x  = 2;
            auto it = v.begin();
            it      = v.insert(it, 3);
            it      = v.insert(it, x);
            it      = v.insert(it, 1);
            CHECK(it == v.begin());
            it = v.insert(v.end(), {5, 6, 7});
            CHECK(it != v.begin());
            CHECK(v == fdn::static_vector<int, 8>{1, 2, 3, 5, 6, 7});
            CHECK(v.size() == 6);
            CHECK(v.max_size() == 8);
        }
        {
            fdn::static_vector<nontrivial_int, 5> v;

            auto it = v.insert(v.end(), {5, 6, 7});
            it      = v.insert(v.begin(), 3);
            it      = v.insert(v.begin() + 2, 1);
            CHECK(v == fdn::static_vector<nontrivial_int, 5>{3, 5, 1, 6, 7});
        }
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{1, 2, 3};
                (void)v.insert(v.begin(), 5);
                int const value = 8;
                (void)v.insert(v.begin() + 2, value);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector{5, 1, 8, 2, 3});
        }
    }

    SECTION("insert range") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 6> v{1, 2, 3, 4};
                fdn::array<int, 3>         a{5, 6, 7};
                (void)v.insert(v.begin() + 2, a.begin(), a.begin() + 2);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector{1, 2, 5, 6, 3, 4});
        }
        {
            constexpr auto v1 = []() {
                fdn::static_vector<nontrivial_int, 3> v;
                fdn::array<nontrivial_int, 3>         a{0, 1, 2};
                (void)v.insert(v.begin(), a.begin(), a.end());
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<nontrivial_int, 3>{0, 1, 2});
        }
        {
            fdn::static_vector<int, 5> v{0, 1, 2};
            fdn::array<int, 3>         a{5, 6, 7};

            auto it = v.insert(v.begin() + 1, a.begin(), a.begin() + 2);
            CHECK(it == v.begin() + 1);
            CHECK(v == fdn::static_vector{0, 5, 6, 1, 2});
            CHECK(v.size() == 5);
        }
    }

    SECTION("emplace") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{0, 1, 2};
                (void)v.emplace(v.begin() + 1, 3);
                (void)v.emplace(v.begin() + 1, 4);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector{0, 4, 3, 1, 2});
        }
        {
            constexpr auto v1 = []() {
                fdn::static_vector<nontrivial_int, 3> v;
                (void)v.emplace(v.begin(), 3);
                (void)v.emplace(v.begin(), 4);
                (void)v.emplace(v.begin(), 2);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<nontrivial_int, 3>{2, 4, 3});
        }
        {
            fdn::static_vector<int, 5> v{0, 1, 2};

            auto it = v.begin() + 1;
            it      = v.emplace(it, 3);
            it      = v.emplace(it, 4);
            CHECK(it == v.begin() + 1);
            CHECK(v == fdn::static_vector{0, 4, 3, 1, 2});
        }
    }

    SECTION("erase one") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 3> v{2, 3, 4};
                (void)v.erase(v.begin());
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 3>{3, 4});
        }
        {
            fdn::static_vector<int, 3> v{2, 3, 4};
            auto                       it = v.erase(v.begin());
            CHECK(it == v.begin());
            CHECK(v == fdn::static_vector<int, 3>{3, 4});
        }
    }

    SECTION("erase range") {
        {
            constexpr auto v1 = []() {
                fdn::static_vector<int, 5> v{0, 1, 2, 3, 4};
                (void)v.erase(v.begin() + 1, v.begin() + 3);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<int, 5>{0, 3, 4});
        }
        {
            constexpr auto v1 = []() {
                fdn::static_vector<nontrivial_int, 6> v{0, 1, 2};
                fdn::array<nontrivial_int, 3>         a{5, 6, 7};
                (void)v.insert(v.end(), a.begin(), a.end());
                (void)v.erase(v.begin() + 2, v.begin() + 4);
                return v;
            }();
            STATIC_REQUIRE(v1 == fdn::static_vector<nontrivial_int, 6>{0, 1, 6, 7});
        }
        {
            fdn::static_vector<int, 8> v1{2, 1, 4, 5, 0, 3};
            auto                       it = v1.erase(v1.begin() + 2, v1.begin() + 4);
            CHECK(it == v1.begin() + 2);
            CHECK(v1 == fdn::static_vector<int, 8>{2, 1, 0, 3});
        }
    }
}