#include <catch2/catch.hpp>

#include <filesystem>

#include <salt/utils/detail/source_location.hpp>

TEST_CASE("salt::utils::source_location", "[salt-utils]") {
    SECTION("it produces a information about the source code, such as file names, line numbers, "
            "and function names") {
        using std::filesystem::path;

        auto const expected_source_location_line          = 17;
        auto const expected_source_location_column        = 45;
        auto const expected_source_location_file_name     = std::string{"source_location-test.cpp"};
        auto const expected_source_location_function_name = std::string{"C_A_T_C_H_T_E_S_T_0"};

        auto const source_location        = salt::source_location::current();
        auto const source_location_line   = source_location.line();
        auto const source_location_column = source_location.column();
        auto const source_location_file_name =
                path(std::string{source_location.file_name()}).filename().string();
        auto const source_location_function_name = std::string{source_location.function_name()};

        REQUIRE(source_location_line == expected_source_location_line);
        REQUIRE(source_location_column == expected_source_location_column);
        REQUIRE(source_location_file_name == expected_source_location_file_name);
        REQUIRE(source_location_function_name == expected_source_location_function_name);
    }
}