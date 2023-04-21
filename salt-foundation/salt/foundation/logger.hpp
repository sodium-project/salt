#pragma once

#include <salt/meta.hpp>

#include <salt/foundation/detail/logger_impl.hpp>
#include <salt/foundation/fast_terminate.hpp>

namespace salt {

namespace output {

static constexpr inline File    file;
static constexpr inline Console console;

} // namespace output

namespace color {

// clang-format off
static constexpr inline Color cyan   = {0  , 200, 240};
static constexpr inline Color white  = {255, 255, 255};
static constexpr inline Color yellow = {231, 231, 31 };
static constexpr inline Color red    = {231, 31 , 31 };
// clang-format on

} // namespace color

namespace log_level {

static constexpr inline Trace   trace   = {.color = color::cyan};
static constexpr inline Debug   debug   = {.color = color::white};
static constexpr inline Warning warning = {.color = color::yellow};
static constexpr inline Error   error   = {.color = color::red};

} // namespace log_level

template <typename Output> detail::Logger<Output>& logger() noexcept {
    static detail::Logger<Output> instance;
    return instance;
}

// clang-format off
template <typename... Args> struct [[maybe_unused]] trace final {
    constexpr explicit
    trace(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<Console>().log(log_level::trace, std::forward_as_tuple(std::move(args)...),
                              location);
    }
    constexpr explicit
    trace(File, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<File>().log(log_level::trace, std::forward_as_tuple(std::move(args)...),
                           location);
    }
};

template <typename... Args> struct [[maybe_unused]] debug final {
    constexpr explicit
    debug(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<Console>().log(log_level::debug, std::forward_as_tuple(std::move(args)...),
                              location);
    }
    constexpr explicit
    debug(File, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<File>().log(log_level::debug, std::forward_as_tuple(std::move(args)...),
                           location);
    }
};

template <typename... Args> struct [[maybe_unused]] warning final {
    constexpr explicit
    warning(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<Console>().log(log_level::warning, std::forward_as_tuple(std::move(args)...),
                              location);
    }
    constexpr explicit
    warning(File, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<File>().log(log_level::warning, std::forward_as_tuple(std::move(args)...),
                           location);
    }
};

template <typename... Args> struct [[maybe_unused]] error final {
    [[noreturn]] constexpr explicit
    error(Args&&... args, source_location location = source_location::current()) noexcept {
        logger<Console>().log(log_level::error, std::forward_as_tuple(std::move(args)...),
                              location);
        fast_terminate();
    }
    [[noreturn]] constexpr explicit
    error(File, Args&&... args, source_location location = source_location::current()) noexcept {
        logger<File>().log(log_level::error, std::forward_as_tuple(std::move(args)...),
                           location);
        fast_terminate();
    }
};
// clang-format on

// clang-format off
template <typename... Args> trace  (Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (Args&&...) -> debug  <Args...>;
template <typename... Args> warning(Args&&...) -> warning<Args...>;
template <typename... Args> error  (Args&&...) -> error  <Args...>;

template <typename... Args> trace  (File, Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (File, Args&&...) -> debug  <Args...>;
template <typename... Args> warning(File, Args&&...) -> warning<Args...>;
template <typename... Args> error  (File, Args&&...) -> error  <Args...>;
// clang-format on

} // namespace salt