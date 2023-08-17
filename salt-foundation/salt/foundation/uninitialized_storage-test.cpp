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

consteval int storage_construct() noexcept {
    fdn::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    std::construct_at(storage.data(), 10);
    int value = storage.data()->value;
    std::destroy_at(storage.data());
    return value;
}
consteval int storage_accessors() noexcept {
    fdn::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    std::construct_at(fdn::get<dummy*>(storage), 20);
    int value = fdn::get<dummy&>(storage).value;
    std::destroy_at(fdn::get<dummy*>(storage));
    return value;
}

TEST_CASE("salt::fdn::uninitialized_storage", "[salt-foundation/uninitialized_storage.hpp]") {
    SECTION("evaluate in constant expression") {
        STATIC_REQUIRE(10 == storage_construct());
        STATIC_REQUIRE(20 == storage_accessors());
    }

    SECTION("construct from fundamental type") {
        fdn::uninitialized_storage<int> storage;

        int* value = std::construct_at(storage.data(), 5);
        REQUIRE(5 == *value);
        REQUIRE( fdn::get<int&>(*value) ==  fdn::get<int&>(storage));
        REQUIRE(*fdn::get<int*>(*value) == *fdn::get<int*>(storage));
        std::destroy_at(storage.data());
    }

    SECTION("construct constant from fundamental type") {
        fdn::uninitialized_storage<int> const storage;

        int const* value = std::construct_at(storage.data(), 8);
        REQUIRE(8 == *value);
        REQUIRE( fdn::get<int const&>(*value) ==  fdn::get<int const&>(storage));
        REQUIRE(*fdn::get<int const*>(*value) == *fdn::get<int const*>(storage));
        std::destroy_at(storage.data());
    }
}