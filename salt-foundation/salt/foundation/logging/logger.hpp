#pragma once
#include <salt/foundation/logging/detail/logger_impl.hpp>
#include <salt/foundation/utility/terminate.hpp>

namespace salt::log {

namespace output {

inline constexpr file_tag    file    = {};
inline constexpr console_tag console = {};

} // namespace output

namespace color {

// clang-format off
inline constexpr ansi_color cyan   = {0  , 200, 240, 255};
inline constexpr ansi_color white  = {255, 255, 255, 255};
inline constexpr ansi_color yellow = {231, 231, 31 , 255};
inline constexpr ansi_color red    = {231, 31 , 31 , 255};
// clang-format on

} // namespace color

namespace log_level {

struct [[nodiscard]] trace_t final {
    static constexpr ansi_color       color  = color::cyan;
    static constexpr std::string_view status = "[TRACE]";
};
struct [[nodiscard]] debug_t final {
    static constexpr ansi_color       color  = color::white;
    static constexpr std::string_view status = "[DEBUG]";
};
struct [[nodiscard]] warning_t final {
    static constexpr ansi_color       color  = color::yellow;
    static constexpr std::string_view status = "[WARNING]";
};
struct [[nodiscard]] error_t final {
    static constexpr ansi_color       color  = color::red;
    static constexpr std::string_view status = "[ERROR]";
};

inline constexpr trace_t   trace;
inline constexpr debug_t   debug;
inline constexpr warning_t warning;
inline constexpr error_t   error;

} // namespace log_level

// clang-format off
template <typename Output>
detail::dummy_logger<Output>& logger() noexcept {
    static detail::dummy_logger<Output> instance;
    return instance;
}
// clang-format on

// clang-format off
template <typename... Args> struct [[maybe_unused]] trace final {
    constexpr explicit
    trace(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<console_tag>().log(log_level::trace, meta::forward_as_tuple(meta::move(args)...),
                                  location);
    }
    constexpr explicit
    trace(file_tag, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<file_tag>().log(log_level::trace, meta::forward_as_tuple(meta::move(args)...),
                               location);
    }
};

template <typename... Args> struct [[maybe_unused]] debug final {
    constexpr explicit
    debug(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<console_tag>().log(log_level::debug, meta::forward_as_tuple(meta::move(args)...),
                                  location);
    }
    constexpr explicit
    debug(file_tag, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<file_tag>().log(log_level::debug, meta::forward_as_tuple(meta::move(args)...),
                               location);
    }
};

template <typename... Args> struct [[maybe_unused]] warning final {
    constexpr explicit
    warning(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<console_tag>().log(log_level::warning, meta::forward_as_tuple(meta::move(args)...),
                                  location);
    }
    constexpr explicit
    warning(file_tag, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<file_tag>().log(log_level::warning, meta::forward_as_tuple(meta::move(args)...),
                               location);
    }
};

template <typename... Args> struct [[maybe_unused]] error final {
    [[noreturn]] constexpr explicit
    error(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<console_tag>().log(log_level::error, meta::forward_as_tuple(meta::move(args)...),
                                  location);
        utility::terminate();
    }
    [[noreturn]] constexpr explicit
    error(file_tag, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<file_tag>().log(log_level::error, meta::forward_as_tuple(meta::move(args)...),
                               location);
        utility::terminate();
    }
};
// clang-format on

// clang-format off
template <typename... Args> trace  (Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (Args&&...) -> debug  <Args...>;
template <typename... Args> warning(Args&&...) -> warning<Args...>;
template <typename... Args> error  (Args&&...) -> error  <Args...>;

template <typename... Args> trace  (file_tag, Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (file_tag, Args&&...) -> debug  <Args...>;
template <typename... Args> warning(file_tag, Args&&...) -> warning<Args...>;
template <typename... Args> error  (file_tag, Args&&...) -> error  <Args...>;
// clang-format on

} // namespace salt::log