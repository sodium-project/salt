#pragma once

#include <salt/meta.hpp>

#include <salt/foundation/detail/logger_impl.hpp>

namespace salt {

namespace color {

// clang-format off
static constexpr inline Color cyan   = {0  , 200, 240};
static constexpr inline Color white  = {255, 255, 255};
static constexpr inline Color yellow = {231, 231, 31 };
static constexpr inline Color red    = {231, 31 , 31 };
// clang-format on

} // namespace color

namespace log_level {

static constexpr inline auto trace   = Trace{.color = color::cyan};
static constexpr inline auto debug   = Debug{.color = color::white};
static constexpr inline auto warning = Warning{.color = color::yellow};
static constexpr inline auto error   = Error{.color = color::red};

} // namespace log_level

template <typename Output> inline detail::Logger<Output>& logger() noexcept {
    static detail::Logger<Output> instance;
    return instance;
}

template <typename... Args> struct [[maybe_unused]] trace final {
    constexpr trace(Args&&... args,
                    source_location location = source_location::current()) noexcept {
        logger<output::Console>().log(log_level::trace, std::forward_as_tuple(std::move(args)...),
                                      location);
    }
};

template <typename... Args> struct [[maybe_unused]] debug final {
    constexpr debug(Args&&... args,
                    source_location location = source_location::current()) noexcept {
        logger<output::Console>().log(log_level::debug, std::forward_as_tuple(std::move(args)...),
                                      location);
    }
};

template <typename... Args> struct [[maybe_unused]] warning final {
    constexpr warning(Args&&... args,
                      source_location location = source_location::current()) noexcept {
        logger<output::Console>().log(log_level::warning, std::forward_as_tuple(std::move(args)...),
                                      location);
    }
};

template <typename... Args> struct [[maybe_unused]] error final {
    [[noreturn]] constexpr error(Args&&... args,
                                 source_location location = source_location::current()) noexcept {
        logger<output::Console>().log(log_level::error, std::forward_as_tuple(std::move(args)...),
                                      location);
        std::abort();
    }
};

// clang-format off
template <typename... Args> trace  (Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (Args&&...) -> debug  <Args...>;
template <typename... Args> warning(Args&&...) -> warning<Args...>;
template <typename... Args> error  (Args&&...) -> error  <Args...>;
// clang-format on

template <typename... Args> struct [[maybe_unused]] ftrace final {
    constexpr ftrace(Args&&... args,
                     source_location location = source_location::current()) noexcept {
        logger<output::File>().log(log_level::trace, std::forward_as_tuple(std::move(args)...),
                                   location);
    }
};

template <typename... Args> struct [[maybe_unused]] fdebug final {
    constexpr fdebug(Args&&... args,
                     source_location location = source_location::current()) noexcept {
        logger<output::File>().log(log_level::debug, std::forward_as_tuple(std::move(args)...),
                                   location);
    }
};

template <typename... Args> struct [[maybe_unused]] fwarning final {
    constexpr fwarning(Args&&... args,
                       source_location location = source_location::current()) noexcept {
        logger<output::File>().log(log_level::warning, std::forward_as_tuple(std::move(args)...),
                                   location);
    }
};

template <typename... Args> struct [[maybe_unused]] ferror final {
    [[noreturn]] constexpr ferror(Args&&... args,
                                  source_location location = source_location::current()) noexcept {
        logger<output::File>().log(log_level::error, std::forward_as_tuple(std::move(args)...),
                                   location);
        std::abort();
    }
};

// clang-format off
template <typename... Args> ftrace  (Args&&...) -> ftrace  <Args...>;
template <typename... Args> fdebug  (Args&&...) -> fdebug  <Args...>;
template <typename... Args> fwarning(Args&&...) -> fwarning<Args...>;
template <typename... Args> ferror  (Args&&...) -> ferror  <Args...>;
// clang-format on

} // namespace salt