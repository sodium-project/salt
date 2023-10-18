#include <catch2/catch.hpp>
#include <salt/foundation/detail/iterator_adapter.hpp>

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

TEST_CASE("salt::fdn::detail::iterator_adapter", "[salt-foundation/iterator_adapter.hpp]") {
    using namespace salt::fdn::detail;

    //using iterator       = iterator_adapter<dummy*, adapter>;
    //using const_iterator = iterator_adapter<dummy const*, adapter>;
    //STATIC_REQUIRE(salt::meta::random_access_iterator<iterator>);
    //STATIC_REQUIRE(salt::meta::random_access_iterator<const_iterator>);
}