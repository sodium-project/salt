#include <catch2/catch.hpp>
#include <salt/foundation.hpp>

#include <memory>

struct dummy {
    int value{};

    constexpr dummy() noexcept {}
    constexpr dummy(int v) noexcept : value{v} {}

    constexpr dummy(dummy const&) noexcept {}
    constexpr dummy(dummy&&) noexcept {}

    constexpr dummy& operator=(dummy const&) noexcept {
        return *this;
    }
    constexpr dummy& operator=(dummy&&) noexcept {
        return *this;
    }

    constexpr ~dummy() {}
};

using namespace salt;
static_assert(not meta::trivial<fdn::uninitialized_storage<int>>);
static_assert(meta::standard_layout<fdn::uninitialized_storage<int>>);
static_assert(not meta::trivially_default_constructible<fdn::uninitialized_storage<int>>);
static_assert(meta::trivially_copyable<fdn::uninitialized_storage<int>>);
static_assert(meta::trivially_copy_constructible<fdn::uninitialized_storage<int>>);
static_assert(meta::trivially_move_constructible<fdn::uninitialized_storage<int>>);
static_assert(meta::trivially_copy_assignable<fdn::uninitialized_storage<int>>);
static_assert(meta::trivially_move_assignable<fdn::uninitialized_storage<int>>);
static_assert(meta::trivially_destructible<fdn::uninitialized_storage<int>>);

static_assert(not meta::trivial<fdn::uninitialized_storage<dummy>>);
static_assert(meta::standard_layout<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_default_constructible<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copyable<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copy_constructible<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_move_constructible<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copy_assignable<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_move_assignable<fdn::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_destructible<fdn::uninitialized_storage<dummy>>);

consteval int constexpr_storage_construct() noexcept {
    fdn::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    std::construct_at(storage.data(), 10);
    int value = storage.data()->value;
    std::destroy_at(storage.data());
    return value;
}

TEST_CASE("salt::fdn::uninitialized_storage", "[salt-foundation/uninitialized_storage.hpp]") {
    SECTION("construct uninitialized storage in constant expression") {
        constexpr int result = constexpr_storage_construct();
        STATIC_REQUIRE(10 == result);
    }

    SECTION("construct uninitialized storage from fundamental type") {
        fdn::uninitialized_storage<int> storage;

        int* value = std::construct_at(storage.data(), 5);
        REQUIRE(5 == *value);
        std::destroy_at(storage.data());
    }

    SECTION("construct constant uninitialized storage from fundamental type") {
        fdn::uninitialized_storage<int> const storage;

        int const* value = std::construct_at(storage.data(), 5);
        REQUIRE(5 == *value);
        std::destroy_at(storage.data());
    }
}