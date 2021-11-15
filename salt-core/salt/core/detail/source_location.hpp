#pragma once

#include <cstdint>

#if !defined(SALT_STDLIB_HAS_NO_SOURCE_LOCATION)
#    include <source_location>
#endif

namespace salt::detail {

#if defined(SALT_HAS_NO_SOURCE_LOCATION)

struct [[nodiscard]] Source_location final {

    // NOTE(Andrii):
    //  This method should be defined as `consteval`, but since `consteval` didn't work in MSVC prior to VS2022
    //  and (Apple)Clang 14, let's stay with `constexpr`.
    static SALT_CONSTEVAL Source_location current(std::uint_least32_t line          = __builtin_LINE(),
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

#endif

} // namespace salt::detail

namespace salt {
#if defined(SALT_HAS_NO_SOURCE_LOCATION)
using source_location = detail::Source_location;
#else
using source_location = std::source_location;
#endif
} // namespace salt