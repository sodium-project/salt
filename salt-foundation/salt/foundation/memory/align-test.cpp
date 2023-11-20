#include <salt/foundation/memory/align.hpp>

#include <catch2/catch.hpp>

using namespace salt::memory;

TEST_CASE("salt::memory::align_offset", "[salt-memory/align.hpp]") {
    auto ptr = reinterpret_cast<void*>(0);
    REQUIRE(align_offset(ptr, 1) == 0u);
    REQUIRE(align_offset(ptr, 16) == 0u);
    ptr = reinterpret_cast<void*>(1);
    REQUIRE(align_offset(ptr, 1) == 0u);
    REQUIRE(align_offset(ptr, 16) == 15u);
    ptr = reinterpret_cast<void*>(8);
    REQUIRE(align_offset(ptr, 4) == 0u);
    REQUIRE(align_offset(ptr, 8) == 0u);
    REQUIRE(align_offset(ptr, 16) == 8u);
    ptr = reinterpret_cast<void*>(16);
    REQUIRE(align_offset(ptr, 16) == 0u);
    ptr = reinterpret_cast<void*>(1025);
    REQUIRE(align_offset(ptr, 16) == 15u);
}

TEST_CASE("salt::memory::is_aligned", "[salt-memory/align.hpp]") {
    auto ptr = reinterpret_cast<void*>(0);
    REQUIRE(is_aligned(ptr, 1));
    REQUIRE(is_aligned(ptr, 8));
    REQUIRE(is_aligned(ptr, 16));
    ptr = reinterpret_cast<void*>(1);
    REQUIRE(is_aligned(ptr, 1));
    REQUIRE(!is_aligned(ptr, 16));
    ptr = reinterpret_cast<void*>(8);
    REQUIRE(is_aligned(ptr, 1));
    REQUIRE(is_aligned(ptr, 4));
    REQUIRE(is_aligned(ptr, 8));
    REQUIRE(!is_aligned(ptr, 16));
    ptr = reinterpret_cast<void*>(16);
    REQUIRE(is_aligned(ptr, 1));
    REQUIRE(is_aligned(ptr, 8));
    REQUIRE(is_aligned(ptr, 16));
    ptr = reinterpret_cast<void*>(1025);
    REQUIRE(is_aligned(ptr, 1));
    REQUIRE(!is_aligned(ptr, 16));
}

TEST_CASE("salt::memory::alignment_for", "[salt-memory/align.hpp]") {
    static_assert(detail::max_alignment >= 8, "test case not working");
    REQUIRE(alignment_for(1) == 1);
    REQUIRE(alignment_for(2) == 2);
    REQUIRE(alignment_for(3) == 2);
    REQUIRE(alignment_for(4) == 4);
    REQUIRE(alignment_for(5) == 4);
    REQUIRE(alignment_for(6) == 4);
    REQUIRE(alignment_for(7) == 4);
    REQUIRE(alignment_for(8) == 8);
    REQUIRE(alignment_for(9) == 8);
    REQUIRE(alignment_for(100) == detail::max_alignment);
}

TEST_CASE("salt::memory::ilog2", "[salt-memory/align.hpp]") {
    SECTION("Check everything up to 2^16") {
        for (std::size_t i = 0; i != 16; ++i) {
            auto power      = 1u << i;
            auto next_power = 2 * power;
            for (auto x = power; x != next_power; ++x)
                CHECK(ilog2(x) == i);
        }
    }

    CHECK(ilog2(std::size_t(1) << 32) == 32);
    CHECK(ilog2((std::size_t(1) << 32) + 44) == 32);
    CHECK(ilog2((std::size_t(1) << 32) + 2048) == 32);

    CHECK(ilog2(std::size_t(1) << 48) == 48);
    CHECK(ilog2((std::size_t(1) << 48) + 44) == 48);
    CHECK(ilog2((std::size_t(1) << 48) + 2048) == 48);

    CHECK(ilog2(std::size_t(1) << 63) == 63);
    CHECK(ilog2((std::size_t(1) << 63) + 44) == 63);
    CHECK(ilog2((std::size_t(1) << 63) + 2063) == 63);
}

TEST_CASE("salt::memory::ilog2_ceil", "[salt-memory/align.hpp]") {
    SECTION("Check everything up to 2^16") {
        for (std::size_t i = 0; i != 16; ++i) {
            auto power = 1u << i;
            CHECK(ilog2_ceil(power) == i);

            auto next_power = 2 * power;
            for (auto x = power + 1; x != next_power; ++x)
                CHECK(ilog2_ceil(x) == i + 1);
        }
    }

    CHECK(ilog2_ceil(std::size_t(1) << 32) == 32);
    CHECK(ilog2_ceil((std::size_t(1) << 32) + 44) == 33);
    CHECK(ilog2_ceil((std::size_t(1) << 32) + 2048) == 33);

    CHECK(ilog2_ceil(std::size_t(1) << 48) == 48);
    CHECK(ilog2_ceil((std::size_t(1) << 48) + 44) == 49);
    CHECK(ilog2_ceil((std::size_t(1) << 48) + 2048) == 49);

    CHECK(ilog2_ceil(std::size_t(1) << 63) == 63);
    CHECK(ilog2_ceil((std::size_t(1) << 63) + 44) == 64);
    CHECK(ilog2_ceil((std::size_t(1) << 63) + 2063) == 64);
}