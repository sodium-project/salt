#include <salt/foundation.hpp>

#include <catch2/catch.hpp>

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

consteval int storage_construct() noexcept {
    fdn::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    fdn::construct_at(storage.data(), 10);
    int value = storage.data()->value;
    fdn::destroy_at(storage.data());
    return value;
}
consteval int storage_accessors() noexcept {
    fdn::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    fdn::construct_at(get<dummy*>(storage), 20);
    int value = get<dummy&>(storage).value;
    fdn::destroy_at(get<dummy*>(storage));
    return value;
}

TEST_CASE("salt::fdn::uninitialized_storage", "[salt-foundation/uninitialized_storage.hpp]") {

    SECTION("evaluate in constant expression") {
        STATIC_REQUIRE(10 == storage_construct());
        STATIC_REQUIRE(20 == storage_accessors());
    }

    SECTION("construct from fundamental type") {
        fdn::uninitialized_storage<int> storage;

        int* value = fdn::construct_at(storage.data(), 5);
        REQUIRE(5 == *value);
        REQUIRE( get<int&>(*value) ==  get<int&>(storage));
        REQUIRE(*get<int*>(*value) == *get<int*>(storage));
        fdn::destroy_at(storage.data());
    }

    SECTION("construct from user-defined type") {
        fdn::uninitialized_storage<dummy> storage;

        fdn::construct_at(storage.data(), 8);
        REQUIRE(8 == get<dummy&>(storage).value);
        REQUIRE(8 == get<dummy*>(storage)->value);
        fdn::destroy_at(storage.data());
    }
}