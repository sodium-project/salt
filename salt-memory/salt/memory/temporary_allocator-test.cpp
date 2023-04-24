#include <catch2/catch.hpp>

#include <salt/memory/std_allocator.hpp>
#include <salt/memory/temporary_allocator.hpp>

#include <vector>

template <typename T>
using Stack_vector = std::vector<T, salt::Std_allocator<T, salt::Temporary_allocator>>;

struct Dummy {
    Dummy() noexcept : x_{1} {
        REQUIRE(1 == x_);
    }

    ~Dummy() {
        if (1 == x_)
            --x_;
        REQUIRE(0 == x_);
    }

    int x() const noexcept {
        return x_;
    }

private:
    int x_;
};

TEST_CASE("salt::Temporary_allocator", "[salt-memory/temporary_allocator.hpp]") {

    SECTION("test temporary allocator") {
        salt::Temporary_allocator allocator;

        auto ptr = allocator.allocate(sizeof(char), alignof(char));
        INFO("ptr = " << ptr << ", max_alignment = " << salt::detail::max_alignment);
        REQUIRE(salt::detail::is_aligned(ptr, alignof(char)));
        REQUIRE_FALSE(salt::detail::is_aligned(ptr, salt::detail::max_alignment));
    }

    SECTION("test temporary vector") {
        salt::Temporary_allocator allocator;

        Stack_vector<Dummy> v{allocator};
        v.emplace_back();

        REQUIRE(1 == v[0].x());
    }
}