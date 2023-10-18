#include <salt/memory.hpp>

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
static_assert(not meta::trivial<memory::uninitialized_storage<int>>);
static_assert(meta::standard_layout<memory::uninitialized_storage<int>>);
static_assert(not meta::trivially_default_constructible<memory::uninitialized_storage<int>>);
static_assert(meta::trivially_copyable<memory::uninitialized_storage<int>>);
static_assert(meta::trivially_copy_constructible<memory::uninitialized_storage<int>>);
static_assert(meta::trivially_move_constructible<memory::uninitialized_storage<int>>);
static_assert(meta::trivially_copy_assignable<memory::uninitialized_storage<int>>);
static_assert(meta::trivially_move_assignable<memory::uninitialized_storage<int>>);
static_assert(meta::trivially_destructible<memory::uninitialized_storage<int>>);

static_assert(not meta::trivial<memory::uninitialized_storage<dummy>>);
static_assert(meta::standard_layout<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_default_constructible<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copyable<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copy_constructible<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_move_constructible<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_copy_assignable<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_move_assignable<memory::uninitialized_storage<dummy>>);
static_assert(not meta::trivially_destructible<memory::uninitialized_storage<dummy>>);

consteval int storage_construct() noexcept {
    memory::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    memory::construct_at(storage.data(), 10);
    int value = storage.data()->value;
    memory::destroy_at(storage.data());
    return value;
}
consteval int storage_accessors() noexcept {
    memory::uninitialized_storage<dummy> storage;
    static_assert(sizeof(storage) == sizeof(dummy));

    memory::construct_at(get<dummy*>(storage), 20);
    int value = get<dummy&>(storage).value;
    memory::destroy_at(get<dummy*>(storage));
    return value;
}

TEST_CASE("salt::memory::uninitialized_storage", "[salt-foundation/uninitialized_storage.hpp]") {
    REQUIRE(true);
    SECTION("evaluate in constant expression") {
        STATIC_REQUIRE(10 == storage_construct());
        STATIC_REQUIRE(20 == storage_accessors());
    }

    SECTION("construct from fundamental type") {
        memory::uninitialized_storage<int> storage;

        int* value = memory::construct_at(storage.data(), 5);
        REQUIRE(5 == *value);
        REQUIRE( get<int&>(*value) ==  get<int&>(storage));
        REQUIRE(*get<int*>(*value) == *get<int*>(storage));
        memory::destroy_at(storage.data());
    }

    SECTION("construct constant from fundamental type") {
        memory::uninitialized_storage<int> const storage;

        int const* value = memory::construct_at(storage.data(), 8);
        REQUIRE(8 == *value);
        REQUIRE( get<int const&>(*value) ==  get<int const&>(storage));
        REQUIRE(*get<int const*>(*value) == *get<int const*>(storage));
        memory::destroy_at(storage.data());
    }
}