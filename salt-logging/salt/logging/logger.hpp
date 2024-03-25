#pragma once

#include <salt/logging/detail/logger_impl.hpp>

namespace salt::log {

namespace out {

inline constexpr to_file    file    = {};
inline constexpr to_console console = {};

} // namespace out

namespace color {

// clang-format off
inline constexpr ansi_color cyan   = {0  , 200, 240, 255};
inline constexpr ansi_color white  = {255, 255, 255, 255};
inline constexpr ansi_color yellow = {231, 231, 31 , 255};
inline constexpr ansi_color red    = {231, 31 , 31 , 255};
// clang-format on

} // namespace color

// clang-format off
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
template <typename Output>
detail::dummy_logger<Output>& logger() noexcept {
    static detail::dummy_logger<Output> instance;
    return instance;
}
#endif
// clang-format on

// clang-format off
template <typename... Args> struct [[maybe_unused]] trace final {
    // Writes the trace log to the console output by default.
    // - If `SALT_LOGGING_FORCE_TO_FILE` is specified, outputs the trace log to a file.
    constexpr explicit trace(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
#    if SALT_LOGGING_FORCE_TO_FILE
        logger<to_file>().log(color::cyan, "[TRACE]", std::forward_as_tuple(std::move(args)...),
                              location);
#    else
        logger<to_console>().log(color::cyan, "[TRACE]", std::forward_as_tuple(std::move(args)...),
                                 location);
#    endif
#endif
    }
    // Explicitly writes the trace log to the output file.
    constexpr explicit trace(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
        logger<to_file>().log(color::cyan, "[TRACE]", std::forward_as_tuple(std::move(args)...),
                              location);
#endif
    }
};

template <typename... Args> struct [[maybe_unused]] debug final {
    // Writes the debug log to the console output by default.
    // - If `SALT_LOGGING_FORCE_TO_FILE` is specified, outputs the debug log to a file.
    constexpr explicit debug(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
#    if SALT_LOGGING_FORCE_TO_FILE
        logger<to_file>().log(color::white, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                               location);
#    else
        logger<to_console>().log(color::white, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                                  location);
#    endif
#endif
    }
    // Explicitly writes the debug log to the output file.
    constexpr explicit debug(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
        logger<to_file>().log(color::white, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                               location);
#endif
    }
};

template <typename... Args> struct [[maybe_unused]] warning final {
    // Writes the warning log to the console output by default.
    // - If `SALT_LOGGING_FORCE_TO_FILE` is specified, outputs the warning log to a file.
    constexpr explicit warning(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
#    if SALT_LOGGING_FORCE_TO_FILE
        logger<to_file>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                               location);
#    else
        logger<to_console>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                                  location);
#    endif
#endif
    }
    // Explicitly writes the warning log to the output file.
    constexpr explicit warning(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
        logger<to_file>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                               location);
#endif
    }
};

template <typename... Args> struct [[maybe_unused]] error final {
    // Writes the error log to the console output by default.
    constexpr explicit error(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
        logger<to_console>().log(color::red, "[ERROR]", std::forward_as_tuple(std::move(args)...),
                                  location);
#endif
        std::terminate();
    }
    // Explicitly writes the error log to the output file.
    constexpr explicit error(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
#if SALT_LOGGING_ENABLED(TRACE) && SALT_LOGGING_ENABLED(ALL)
        logger<to_file>().log(color::red, "[ERROR]", std::forward_as_tuple(std::move(args)...),
                               location);
#endif
        std::terminate();
    }
};
// clang-format on

// clang-format off
template <typename... Args> trace  (Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (Args&&...) -> debug  <Args...>;
template <typename... Args> warning(Args&&...) -> warning<Args...>;
template <typename... Args> error  (Args&&...) -> error  <Args...>;

template <typename... Args> trace  (to_file, Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (to_file, Args&&...) -> debug  <Args...>;
template <typename... Args> warning(to_file, Args&&...) -> warning<Args...>;
template <typename... Args> error  (to_file, Args&&...) -> error  <Args...>;
// clang-format on

} // namespace salt::log