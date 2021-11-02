#pragma once

#include <cstdint>

#if !defined(__cpp_consteval)

namespace salt::detail {

/*

source_location synopsis

namespace std {

struct source_location
{
// source_location construction
static consteval source_location current() noexcept;
constexpr source_location() noexcept;

// source_location field access
constexpr uint_least32_t line() const noexcept;
constexpr uint_least32_t column() const noexcept;
constexpr const char* file_name() const noexcept;
constexpr const char* function_name() const noexcept;

private:
uint_least32_t line_;
uint_least32_t column_;
const char* file_name_;
const char* function_name_;
};

} // namespace std

*/

struct [[nodiscard]] Source_location final {

    // consteval is broken in MSVC before VS2022 and (Apple)Clang 13.
    static constexpr Source_location current(std::uint_least32_t line          = __builtin_LINE(),
                                             std::uint_least32_t column        = __builtin_COLUMN(),
                                             char const*         file_name     = __builtin_FILE(),
                                             char const*         function_name = __builtin_FUNCTION()) noexcept {
        Source_location source{};
        source.line_          = line;
        source.column_        = column;
        source.file_name_     = file_name;
        source.function_name_ = function_name;

        return source;
    }

    constexpr Source_location() noexcept = default;

    // Source_location field access
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
    std::uint_least32_t line_;
    std::uint_least32_t column_;
    char const*         file_name_{""};
    char const*         function_name_{""};
};

} // namespace salt::detail

#else
#    include <source_location>
#endif

namespace salt {
#if !defined(__cpp_consteval)
using source_location = detail::Source_location;
#else
using source_location = std::source_location;
#endif
} // namespace salt