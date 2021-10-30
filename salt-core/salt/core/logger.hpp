#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <chrono>
#include <concepts>
#include <ctime>
#include <filesystem>
#include <string_view>
#include <type_traits>

#include <salt/core/detail/source_location.hpp>

namespace salt {

namespace detail {

inline constexpr fmt::text_style crimson{fmt::fg(fmt::color::crimson)};
inline constexpr fmt::text_style cyan{fmt::fg(fmt::color::cyan)};
inline constexpr fmt::text_style green{fmt::fg(fmt::color::dark_sea_green)};
inline constexpr fmt::text_style red{fmt::fg(fmt::color::red)};
inline constexpr fmt::text_style blue{fmt::fg(fmt::color::slate_blue)};
inline constexpr fmt::text_style yellow{fmt::fg(fmt::color::yellow)};

#if (defined(__cpp_consteval) && (!FMT_MSC_VER || _MSC_FULL_VER >= 193030704))
// clang-format off
// consteval is broken in MSVC before VS2022 and Apple clang 13.
template <typename T>
concept fmt_string_like = std::convertible_to<T, fmt::format_string<T>>;
// clang-format on
#else
// clang-format off
template <typename T>
concept fmt_string_like = std::is_convertible_v<T, std::string_view>;
// clang-format on
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

auto as_local(auto const now) noexcept {
    using std::chrono::system_clock;
    auto const time = system_clock::to_time_t(now);
    return std::localtime(&time);
}

#pragma clang diagnostic pop

std::string to_string(auto const time) {
    return fmt::format("{:%F %T}", *time);
}

std::string to_string(source_location const& source) {
    using std::filesystem::path;
    return fmt::format("{}:{}:{}", path(source.file_name()).filename().string(), source.function_name(), source.line());
}

} // namespace detail

template <detail::fmt_string_like String, typename... Args> struct [[nodiscard]] debug final {

    debug(String message, Args&&... args, source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print("{} [DEBUG] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
    }
};

template <detail::fmt_string_like String, typename... Args> debug(String, Args&&...) -> debug<String, Args...>;

} // namespace salt