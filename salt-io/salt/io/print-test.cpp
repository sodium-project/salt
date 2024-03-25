#include <salt/io.hpp>

#include <catch2/catch.hpp>

struct test_io_device {
    using char_type = char;

    std::string buffer;
};

template <typename Iter>
constexpr void write(test_io_device& device, Iter begin, Iter end) noexcept {
    device.buffer.assign(begin, end);
}

TEST_CASE("salt::io::print", "[salt-io/print.hpp]") {
    using namespace salt;

    SECTION("string literal") {
        test_io_device device;

        io::print(device, "test");
        CHECK(device.buffer == std::string{"test"});
    }

    SECTION("integer values") {
        test_io_device device;

        int a = 1;
        int b = 2;
        io::print(device, a, b, 3);
        CHECK(device.buffer == std::string{"123"});
    }
}

TEST_CASE("salt::io::println", "[salt-io/print.hpp]") {
    using namespace salt;

    SECTION("string literal") {
        test_io_device device;

        io::println(device, "test");
        CHECK(device.buffer == std::string{"test\n"});
    }

    SECTION("integer values") {
        test_io_device device;

        int a = 1;
        int b = 2;
        io::println(device, a, b, 3);
        CHECK(device.buffer == std::string{"123\n"});
    }
}