#include <catch2/catch.hpp>

#include <salt/utils.hpp>

TEST_CASE("salt::as_local", "[salt-utils/as_local.hpp]") {
    SECTION("it produces a time as local") {
        using std::chrono::system_clock;

        SALT_DISABLE_WARNING_PUSH
        SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS

        auto const current_time        = system_clock::now();
        auto const as_time_t           = system_clock::to_time_t(current_time);
        auto const expected_local_time = std::localtime(&as_time_t);

        auto const local_time = salt::as_local(current_time);
        // clang-format off
        REQUIRE(local_time->tm_year  == expected_local_time->tm_year);
        REQUIRE(local_time->tm_mon   == expected_local_time->tm_mon);
        REQUIRE(local_time->tm_mday  == expected_local_time->tm_mday);
        REQUIRE(local_time->tm_hour  == expected_local_time->tm_hour);
        REQUIRE(local_time->tm_min   == expected_local_time->tm_min);
        REQUIRE(local_time->tm_isdst == expected_local_time->tm_isdst);
        // clang-format on

        SALT_DISABLE_WARNING_POP
    }
}