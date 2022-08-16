#include <catch2/catch.hpp>

#include <salt/foundation/detail/strip_path.hpp>
#include <string.h>

using namespace salt::detail;

TEST_CASE("salt::strip_path", "[salt-foundation/strip_path.hpp]") {
    SECTION("strip empty path") {
        char path[] = "";
        REQUIRE(strcmp("", strip_path(path)) == 0);
    }

    SECTION("strip absolute path") {
        {
            char absolute_path[] = "C:\\Program Files\\LLVM\\include\\llvm-c\\lto.h";
            char expected_path[] = "lto.h";
            REQUIRE(strcmp(expected_path, strip_path(absolute_path)) == 0);
        }
        {
            char absolute_path[] = "Users/user/LLVM/include/llvm-c/lto.h";
            char expected_path[] = "lto.h";
            REQUIRE(strcmp(expected_path, strip_path(absolute_path)) == 0);
        }
    }

    SECTION("strip relative path") {
        {
            char relative_path[] = "..\\include\\llvm-c\\lto.h";
            char expected_path[] = "lto.h";
            REQUIRE(strcmp(expected_path, strip_path(relative_path)) == 0);
        }
        {
            char relative_path[] = "../include/llvm-c/lto.h";
            char expected_path[] = "lto.h";
            REQUIRE(strcmp(expected_path, strip_path(relative_path)) == 0);
        }
    }

    SECTION("remove file extension") {
        char filename[] = "example.cpp";
        auto basename   = salt::detail::basename(filename, last_dot_of(filename));
        REQUIRE(std::string{"example"} == to_string(basename));
    }
}