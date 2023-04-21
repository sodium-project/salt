#include <catch2/catch.hpp>

#include <salt/memory/detail/debug_helpers.hpp>

#include <salt/memory/debugging.hpp>

using namespace salt;
using namespace salt::detail;

TEST_CASE("salt::detail::debug_fill", "[salt-memory/debug_helpers.hpp]") {
    debug_magic magic_values[10];
    for (auto& magic_value : magic_values)
        magic_value = debug_magic::freed_memory;

    debug_fill(magic_values, sizeof(magic_values), debug_magic::new_memory);
#if SALT_MEMORY_DEBUG_FILL
    for (auto magic_value : magic_values)
        REQUIRE(magic_value == debug_magic::new_memory);
#else
    for (auto magic_value : magic_values)
        REQUIRE(magic_value == debug_magic::freed_memory);
#endif
}

TEST_CASE("salt::detail::debug_is_filled", "[salt-memory/debug_helpers.hpp]") {
    debug_magic magic_values[10];
    for (auto& magic_value : magic_values)
        magic_value = debug_magic::freed_memory;

    REQUIRE(debug_is_filled(magic_values, sizeof(magic_values), debug_magic::freed_memory) ==
            nullptr);

    magic_values[5] = debug_magic::new_memory;
    auto ptr        = static_cast<debug_magic*>(
            debug_is_filled(magic_values, sizeof(magic_values), debug_magic::freed_memory));
#if SALT_MEMORY_DEBUG_FILL
    REQUIRE(ptr == magic_values + 5);
#else
    REQUIRE(ptr == nullptr);
#endif
}

TEST_CASE("salt::detail::debug_fill_new/free", "[salt-memory/debug_helpers.hpp]") {
    debug_magic magic_values[10];

    auto result = debug_fill_new(magic_values, 8 * sizeof(debug_magic), sizeof(debug_magic));
    auto offset = static_cast<debug_magic*>(result) - magic_values;
    auto expected_offset = static_cast<std::int32_t>(debug_fence_size ? sizeof(debug_magic) : 0);
    REQUIRE(offset == expected_offset);

#if SALT_MEMORY_DEBUG_FILL
#    if SALT_MEMORY_DEBUG_FENCE
    REQUIRE(magic_values[0] == debug_magic::fence_memory);
    REQUIRE(magic_values[9] == debug_magic::fence_memory);
    const auto start = 1;
#    else
    const auto start = 0;
#    endif
    for (auto i = start; i < start + 8; ++i)
        REQUIRE(magic_values[i] == debug_magic::new_memory);
#endif

    result = debug_fill_free(result, 8 * sizeof(debug_magic), sizeof(debug_magic));
    REQUIRE(static_cast<debug_magic*>(result) == magic_values);

#if SALT_MEMORY_DEBUG_FILL
#    if SALT_MEMORY_DEBUG_FENCE
    REQUIRE(magic_values[0] == debug_magic::fence_memory);
    REQUIRE(magic_values[9] == debug_magic::fence_memory);
#    endif
    for (auto i = start; i < start + 8; ++i)
        REQUIRE(magic_values[i] == debug_magic::freed_memory);
#endif
}
