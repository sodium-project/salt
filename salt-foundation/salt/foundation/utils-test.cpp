#include <catch2/catch.hpp>

#include <salt/foundation.hpp>

TEST_CASE("salt::foundation::utils::as_local", "[salt-foundation]") {
    SECTION("it produces a time as local") {
        using std::chrono::system_clock;

        SALT_DISABLE_WARNING_PUSH
        SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS

        auto const current_time        = system_clock::now();
        auto const as_time_t           = system_clock::to_time_t(current_time);
        auto const expected_local_time = std::localtime(&as_time_t);

        auto const local_time = salt::as_local(current_time);
        REQUIRE(local_time->tm_year == expected_local_time->tm_year);
        REQUIRE(local_time->tm_mon == expected_local_time->tm_mon);
        REQUIRE(local_time->tm_mday == expected_local_time->tm_mday);
        REQUIRE(local_time->tm_hour == expected_local_time->tm_hour);
        REQUIRE(local_time->tm_min == expected_local_time->tm_min);
        REQUIRE(local_time->tm_isdst == expected_local_time->tm_isdst);

        SALT_DISABLE_WARNING_POP
    }
}

TEST_CASE("salt::foundation::utils::to_string", "[salt-foundation]") {
    SECTION("it converts time to std::string") {
        using std::chrono::system_clock;

        SALT_DISABLE_WARNING_PUSH
        SALT_DISABLE_WARNING_DEPRECATED_DECLARATIONS

        std::tm tm{};
        tm.tm_year       = 2021 - 1900; // 2021
        tm.tm_mon        = 11 - 1;      // November
        tm.tm_mday       = 18;          // 18th
        tm.tm_hour       = 01;
        tm.tm_min        = 16;
        tm.tm_isdst      = 0; // Not daylight saving
        std::time_t time = std::mktime(&tm);

        auto const local_time   = std::localtime(&time);
        auto const expected_out = std::string{"2021-11-18 01:16:00"};

        auto const out = salt::to_string(local_time);
        REQUIRE(out == expected_out);

        SALT_DISABLE_WARNING_POP
    }

    SECTION("it produces a source location as std::string") {
        auto const expected_location = std::string{"utils-test.cpp:____C_A_T_C_H____T_E_S_T____3:55"};
        auto const location          = salt::to_string(salt::source_location::current());
        REQUIRE(location == expected_location);
    }
}