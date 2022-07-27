#pragma once
#include <string_view>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>

#include <salt/meta.hpp>

#include <salt/foundation/detail/as_local.hpp>
#include <salt/foundation/detail/to_string.hpp>

namespace salt {

namespace detail {

inline constexpr fmt::text_style crimson = fmt::fg(fmt::color::crimson);
inline constexpr fmt::text_style cyan    = fmt::fg(fmt::color::cyan);
inline constexpr fmt::text_style green   = fmt::fg(fmt::color::dark_sea_green);
inline constexpr fmt::text_style red     = fmt::fg(fmt::color::red);
inline constexpr fmt::text_style blue    = fmt::fg(fmt::color::slate_blue);
inline constexpr fmt::text_style yellow  = fmt::fg(fmt::color::yellow);

// clang-format off
template <typename T>
concept string_like = convertible_to<T, std::string_view>;
// clang-format on

} // namespace detail

template <detail::string_like String, typename... Args> struct [[maybe_unused]] trace final {

    trace(String                 message, Args&&... args,
          source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print(detail::blue, "{} [TRACE] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
    }
};

template <detail::string_like String, typename... Args> struct [[maybe_unused]] debug final {

    debug(String                 message, Args&&... args,
          source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print("{} [DEBUG] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
    }
};

template <detail::string_like String, typename... Args> struct [[maybe_unused]] info final {

    info(String                 message, Args&&... args,
         source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print(detail::green, "{} [INFO] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
    }
};

template <detail::string_like String, typename... Args> struct [[maybe_unused]] warning final {

    warning(String                 message, Args&&... args,
            source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print(detail::yellow, "{} [WARNING] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
    }
};

template <detail::string_like String, typename... Args> struct [[maybe_unused]] error final {

    [[noreturn]] error(String                 message, Args&&... args,
                       source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print(detail::red, "{} [ERROR] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
        std::abort();
    }
};

template <detail::string_like String, typename... Args> struct [[maybe_unused]] critical final {

    critical(String                 message, Args&&... args,
             source_location const& source = source_location::current()) noexcept {
        using std::chrono::system_clock;
        // clang-format off
        fmt::print(detail::crimson, "{} [CRITICAL] {} {}\n",
                   detail::to_string(detail::as_local(system_clock::now())),
                   detail::to_string(source),
                   fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
        // clang-format on
    }
};

// clang-format off
template <detail::string_like String, typename... Args> trace   (String, Args&&...) -> trace   <String, Args...>;
template <detail::string_like String, typename... Args> debug   (String, Args&&...) -> debug   <String, Args...>;
template <detail::string_like String, typename... Args> info    (String, Args&&...) -> info    <String, Args...>;
template <detail::string_like String, typename... Args> warning (String, Args&&...) -> warning <String, Args...>;
template <detail::string_like String, typename... Args> error   (String, Args&&...) -> error   <String, Args...>;
template <detail::string_like String, typename... Args> critical(String, Args&&...) -> critical<String, Args...>;
// clang-format on

} // namespace salt