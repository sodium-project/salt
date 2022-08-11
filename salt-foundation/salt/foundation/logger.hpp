#pragma once
#include <string_view>

#include <salt/meta.hpp>

#include <salt/foundation/detail/as_local.hpp>
#include <salt/foundation/detail/to_string.hpp>

namespace salt {

template <typename... Args> struct [[maybe_unused]] trace final {
    trace(Args&&... args, source_location source = source_location::current()) noexcept {
        fast_io::posix_tzset();
        println(fast_io::out(), "[TRACE] ",
                local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime)), " | ",
                detail::to_string(source), " | ", std::forward<Args>(args)...);
    }
};

template <typename... Args> struct [[maybe_unused]] debug final {
    debug(Args&&... args, source_location source = source_location::current()) noexcept {
        fast_io::posix_tzset();
        println(fast_io::out(), "[DEBUG] ",
                local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime)), " | ",
                detail::to_string(source), " | ", std::forward<Args>(args)...);
    }
};

template <typename... Args> struct [[maybe_unused]] info final {
    info(Args&&... args, source_location source = source_location::current()) noexcept {
        fast_io::posix_tzset();
        println(fast_io::out(), "[INFO] ",
                local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime)), " | ",
                detail::to_string(source), " | ", std::forward<Args>(args)...);
    }
};

template <typename... Args> struct [[maybe_unused]] warning final {
    warning(Args&&... args, source_location source = source_location::current()) noexcept {
        fast_io::posix_tzset();
        println(fast_io::out(), "[WARNING] ",
                local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime)), " | ",
                detail::to_string(source), " | ", std::forward<Args>(args)...);
    }
};

template <typename... Args> struct [[maybe_unused]] error final {
    [[noreturn]] error(Args&&... args,
                       source_location source = source_location::current()) noexcept {
        fast_io::posix_tzset();
        println(fast_io::out(), "[ERROR] ",
                local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime)), " | ",
                detail::to_string(source), " | ", std::forward<Args>(args)...);
        std::abort();
    }
};

template <typename... Args> struct [[maybe_unused]] critical final {
    critical(Args&&... args, source_location source = source_location::current()) noexcept {
        fast_io::posix_tzset();
        println(fast_io::out(), "[CRITICAL] ",
                local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime)), " | ",
                detail::to_string(source), " | ", std::forward<Args>(args)...);
    }
};

// clang-format off
template <typename... Args> trace   (Args&&...) -> trace   <Args...>;
template <typename... Args> debug   (Args&&...) -> debug   <Args...>;
template <typename... Args> info    (Args&&...) -> info    <Args...>;
template <typename... Args> warning (Args&&...) -> warning <Args...>;
template <typename... Args> error   (Args&&...) -> error   <Args...>;
template <typename... Args> critical(Args&&...) -> critical<Args...>;
// clang-format on

} // namespace salt