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

struct A {
    virtual void f() noexcept {}
    virtual ~A() {}
};
struct B {
    virtual void g() noexcept {}
    virtual ~B() {}
};

struct C : A, B {};

void foo() noexcept {}

TEST_CASE("salt::io::address", "[salt-io/manipulators.hpp]") {
    using namespace salt;

    SECTION("integer values") {
        test_io_device device;

        int a = 1;
        io::print(device, a, " | ", io::address(a));
        CHECK(device.buffer.starts_with("1 | 0x"));
    }

    SECTION("basic pointers") {
        test_io_device device;

        int  a = 1;
        int* p = &a;
        io::print(device, *p, " | ", io::address(p));
        CHECK(device.buffer.starts_with("1 | 0x"));
    }

    SECTION("function pointers") {
        test_io_device device;

        io::print(device, io::address(foo));
        CHECK(device.buffer.starts_with("0x"));
    }

    SECTION("member function pointers") {
        test_io_device device;

        void (C::*downcptr)() noexcept = &B::g;
        io::print(device, io::address(downcptr));
        CHECK(device.buffer.starts_with("0x"));
    }
}

TEST_CASE("salt::io::c_str", "[salt-io/manipulators.hpp]") {
    using namespace salt;
    test_io_device device;

    char const* p = "hello";
    io::print(device, io::c_str(p));
    CHECK(device.buffer == std::string{"hello"});
}