#pragma once
#include <salt/config.hpp>

#include <cstdint>
#include <fast_io.h>
#include <fast_io_device.h>

#if defined(__cpp_lib_source_location)
#    include <source_location>
#else
#    include <salt/logging/detail/strip_path.hpp>
namespace salt::detail {

struct [[nodiscard]] source_location final {

    static consteval source_location
    current(std::uint_least32_t line          = __builtin_LINE(),
            std::uint_least32_t column        = __builtin_COLUMN(),
            char const*         file_name     = __builtin_FILE(),
            char const*         function_name = __builtin_FUNCTION()) noexcept {
        source_location source{};
        source.line_          = line;
        source.column_        = column;
        source.file_name_     = file_name;
        source.function_name_ = function_name;

        return source;
    }

    constexpr source_location() noexcept = default;

    constexpr std::uint_least32_t line() const noexcept {
        return line_;
    }

    constexpr std::uint_least32_t column() const noexcept {
        return column_;
    }

    constexpr char const* file_name() const noexcept {
        return file_name_;
    }

    constexpr char const* function_name() const noexcept {
        return function_name_;
    }

private:
    std::uint_least32_t line_          = {};
    std::uint_least32_t column_        = {};
    char const*         file_name_     = "";
    char const*         function_name_ = "";
};

} // namespace salt::detail

namespace fast_io {

struct salt_source_location_scatter {
    basic_io_scatter_t<char> file_name;
    basic_io_scatter_t<char> function_name;
    std::uint_least32_t      line;
    std::uint_least32_t      column;
};

namespace details {

inline constexpr std::size_t
print_reserve_size_source_location_impl(salt_source_location_scatter location) noexcept {
    constexpr auto reserve_size = print_reserve_size(io_reserve_type<char, std::uint_least32_t>);
    constexpr auto total_size   = (reserve_size * 2 + 3);
    return intrinsics::add_or_overflow_die_chain(location.file_name.len, location.function_name.len,
                                                 total_size);
}

inline constexpr char*
print_reserve_define_source_location_impl(char*                        it,
                                          salt_source_location_scatter location) noexcept {
    constexpr auto io_reserve = io_reserve_type<char, std::uint_least32_t>;
    *(it = non_overlapped_copy_n(location.file_name.base, location.file_name.len, it)) = ':';
    *(it = print_reserve_define(io_reserve, ++it, location.line))                      = ':';
    *(it = print_reserve_define(io_reserve, ++it, location.column))                    = ':';
    return non_overlapped_copy_n(location.function_name.base, location.function_name.len, ++it);
}

inline constexpr salt_source_location_scatter
print_alias_define_source_location_impl(salt::detail::source_location location) noexcept {
    using salt::detail::strip_path;
    return {{strip_path(location.file_name()), cstr_len(strip_path(location.file_name()))},
            {location.function_name(), cstr_len(location.function_name())},
            location.line(),
            location.column()};
}

} // namespace details

inline constexpr std::size_t
print_reserve_size(io_reserve_type_t<char, salt_source_location_scatter>,
                   salt_source_location_scatter location) noexcept {
    return details::print_reserve_size_source_location_impl(location);
}

inline constexpr char* print_reserve_define(io_reserve_type_t<char, salt_source_location_scatter>,
                                            char*                        iter,
                                            salt_source_location_scatter location) noexcept {
    return details::print_reserve_define_source_location_impl(iter, location);
}

inline constexpr salt_source_location_scatter
print_alias_define(io_alias_t, salt::detail::source_location location) noexcept {
    return details::print_alias_define_source_location_impl(location);
}

namespace manipulators {
inline constexpr auto cur_src_loc(salt::detail::source_location location =
                                          salt::detail::source_location::current()) noexcept {
    return location;
}
} // namespace manipulators
} // namespace fast_io
#endif

namespace salt::fdn {
#if defined(__cpp_lib_source_location)
using source_location = std::source_location;
#else
using source_location = detail::source_location;
#endif
} // namespace salt::fdn