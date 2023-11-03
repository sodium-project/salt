#include <salt/utility/iterator_adapter.hpp>

#include <catch2/catch.hpp>

struct dummy {
    int a = 1;
};

struct adapter {
    constexpr int& operator()(dummy& d) const noexcept {
        return d.a;
    }
    constexpr int const& operator()(dummy const& d) const noexcept {
        return d.a;
    }
};

TEST_CASE("salt::utility::iterator_adapter", "[salt-utility/iterator_adapter.hpp]") {
    using namespace salt::utility;

    using iterator       = iterator_adapter<dummy*, adapter>;
    using const_iterator = iterator_adapter<dummy const*, adapter>;
    STATIC_REQUIRE(salt::meta::random_access_iterator<iterator>);
    STATIC_REQUIRE(salt::meta::random_access_iterator<const_iterator>);
}