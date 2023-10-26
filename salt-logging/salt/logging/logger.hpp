#pragma once
#include <salt/logging/detail/logger_impl.hpp>

namespace salt::log {

namespace output {

static constexpr inline file_tag    file    = {};
static constexpr inline console_tag console = {};

} // namespace output

namespace color {

// clang-format off
static constexpr inline print_color cyan   = {0  , 200, 240, 255};
static constexpr inline print_color white  = {255, 255, 255, 255};
static constexpr inline print_color yellow = {231, 231, 31 , 255};
static constexpr inline print_color red    = {231, 31 , 31 , 255};
// clang-format on

} // namespace color

namespace log_level {

// clang-format off
static constexpr inline trace_tag   trace   = {.color = color::cyan  };
static constexpr inline debug_tag   debug   = {.color = color::white };
static constexpr inline warning_tag warning = {.color = color::yellow};
static constexpr inline error_tag   error   = {.color = color::red   };
// clang-format on

} // namespace log_level

// clang-format off
template <typename Output>
detail::default_logger<Output>& logger() noexcept {
    static detail::default_logger<Output> instance;
    return instance;
}
// clang-format on

// clang-format off
template <typename... Args> struct [[maybe_unused]] trace final {
    constexpr explicit
    trace(Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<console_tag>().log(log_level::trace, std::forward_as_tuple(meta::move(args)...),
                                  location);
    }
    constexpr explicit
    trace(file_tag, Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<file_tag>().log(log_level::trace, std::forward_as_tuple(meta::move(args)...),
                               location);
    }
};

template <typename... Args> struct [[maybe_unused]] debug final {
    constexpr explicit
    debug(Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<console_tag>().log(log_level::debug, std::forward_as_tuple(meta::move(args)...),
                                  location);
    }
    constexpr explicit
    debug(file_tag, Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<file_tag>().log(log_level::debug, std::forward_as_tuple(meta::move(args)...),
                               location);
    }
};

template <typename... Args> struct [[maybe_unused]] warning final {
    constexpr explicit
    warning(Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<console_tag>().log(log_level::warning, std::forward_as_tuple(meta::move(args)...),
                                  location);
    }
    constexpr explicit
    warning(file_tag, Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<file_tag>().log(log_level::warning, std::forward_as_tuple(meta::move(args)...),
                               location);
    }
};

template <typename... Args> struct [[maybe_unused]] error final {
    [[noreturn]] constexpr explicit
    error(Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<console_tag>().log(log_level::error, std::forward_as_tuple(meta::move(args)...),
                                  location);
        __builtin_trap();
    }
    [[noreturn]] constexpr explicit
    error(file_tag, Args&&... args, fdn::source_location location = fdn::source_location::current()) noexcept {
        logger<file_tag>().log(log_level::error, std::forward_as_tuple(meta::move(args)...),
                               location);
        __builtin_trap();
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