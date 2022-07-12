#include <catch2/catch.hpp>

#include <salt/foundation.hpp>

struct [[nodiscard]] Dummy final {
    int* pointer = nullptr;
};

TEST_CASE("salt::Uninitialized_storage", "[salt-foundation/uninitialized_storage.hpp]") {
    SECTION("it can be constructed from fundamental type") {
        auto storage = salt::Uninitialized_storage_for<int>{};
        storage.construct<int>(2022);

        auto expected_value = int{2022};
        REQUIRE(expected_value == *storage.get<int>());

        storage.destruct<int>();
    }

    SECTION("it can be constructed from user-defined type") {
        auto expected_value = int{2022};

        auto storage = salt::Uninitialized_storage_for<Dummy>{};
        storage.construct<Dummy>(Dummy{&expected_value});

        REQUIRE(expected_value == *(*storage.get<Dummy>()).pointer);

        storage.destruct<Dummy>();
    }
}