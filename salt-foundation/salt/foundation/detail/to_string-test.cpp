#include <catch2/catch.hpp>

#include <salt/foundation.hpp>

using namespace salt::detail;

TEST_CASE("salt::to_string", "[salt-foundation/to_string.hpp]") {
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

        auto const out = to_string(local_time);
        REQUIRE(out == expected_out);

        SALT_DISABLE_WARNING_POP
    }

    SECTION("it produces a source location as std::string") {
        auto const expected_location = std::string{"to_string-test.cpp:C_A_T_C_H_T_E_S_T_0:34"};
        auto const location          = to_string(salt::source_location::current());
        REQUIRE(location == expected_location);
    }
}