#pragma once
#include <string_view>

#include <fast_io.h>
#include <fast_io_device.h>
#include <salt/foundation/detail/source_location.hpp>

// NOTE:
//  This header is such a mess, it will need to be revisited in the future and probably refactored.
//  Maybe add multithreading support as well. But at this stage, let it be as it is.
// TODO:
//  Revisit it.
namespace salt {

struct [[nodiscard]] File    final {};
struct [[nodiscard]] Console final {};

namespace color {

struct [[nodiscard]] Color final {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
};

constexpr std::string_view end() noexcept {
    return "\033[0;00m";
}

constexpr std::size_t print_reserve_size(fast_io::io_reserve_type_t<char, Color>) {
    using namespace fast_io;
    constexpr auto reserve_size = print_reserve_size(io_reserve_type<char, std::uint8_t>);
    constexpr auto total_size   = (reserve_size * 3 + 10);
    return total_size;
}

template <std::contiguous_iterator Iter>
constexpr Iter print_reserve_define(fast_io::io_reserve_type_t<std::iter_value_t<Iter>, Color>,
                                    Iter it, Color color) {
    using namespace fast_io;
    *(it = details::non_overlapped_copy_n("\033[38;2", sizeof "\033[38;2" - 1, it))  = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.r)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.g)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.b)) = 'm';
    return ++it;
}

} // namespace color
namespace log_level {

struct [[nodiscard]] Trace final {
    color::Color color;
    constexpr operator std::string_view() const noexcept {
        return "[TRACE]";
    }
};
struct [[nodiscard]] Debug final {
    color::Color color;
    constexpr operator std::string_view() const noexcept {
        return "[DEBUG]";
    }
};
struct [[nodiscard]] Warning final {
    color::Color color;
    constexpr operator std::string_view() const noexcept {
        return "[WARNING]";
    }
};
struct [[nodiscard]] Error final {
    color::Color color;
    constexpr operator std::string_view() const noexcept {
        return "[ERROR]";
    }
};

} // namespace log_level

// clang-format off
template <typename Color>
concept log_color = 
    std::is_class_v<Color>            and
    std::default_initializable<Color> and
    requires(Color color) {
        { prvalue(color.r) } -> std::same_as<std::uint8_t>;
        { prvalue(color.g) } -> std::same_as<std::uint8_t>;
        { prvalue(color.b) } -> std::same_as<std::uint8_t>;
        requires sizeof(std::uint8_t) * 3 == sizeof(color);
        // Requires that constructing a Color from `r, g, b` is a valid expression
        Color{color.r, color.g, color.b};
    };

template <typename Type>
concept log_type = 
    std::is_class_v<Type>                       and
    std::default_initializable<Type>            and
    std::convertible_to<Type, std::string_view> and
    requires(Type type) {
        { prvalue(type.color) } -> log_color;
        // Requires that constructing a Type from `color` is a valid expression
        Type{type.color};
    };
// clang-format on

namespace detail {
template <typename Output> struct [[maybe_unused]] Logger;
} // namespace detail

template <typename Output> detail::Logger<Output>& logger() noexcept;

namespace detail {

inline auto get_local_time() noexcept {
    [[maybe_unused]] static bool once = ([] { fast_io::posix_tzset(); }(), true);
    return local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime));
}

#define SALT_PRINT_LOGGER_HEADER_TO(output_file)                                                   \
    println(output_file, left("+", 36, '-'), left("+", 63, '-'), left("+", 13, '-'),               \
            left("+", 32, '-'));                                                                   \
    println(output_file, "| ", left("Date / Time", 33), " | ", left("Location", 60), " | ",        \
            left("Level", 10), " | Message");                                                      \
    println(output_file, left("+", 36, '-'), left("+", 63, '-'), left("+", 13, '-'),               \
            left("+", 32, '-'));

template <> struct [[maybe_unused]] Logger<File> final {
    constexpr Logger(Logger const&)            = delete;
    constexpr Logger(Logger&&)                 = delete;
    constexpr Logger& operator=(Logger const&) = delete;
    constexpr Logger& operator=(Logger&&)      = delete;

    // clang-format off
    template <log_type Type, typename... Args>
    constexpr void log(Type                  type,
                       std::tuple<Args&&...> tuple_args,
                       source_location       location) noexcept {
        using namespace fast_io::mnp;
        [[maybe_unused]] fast_io::io_flush_guard guard{output_file_};
        std::apply(
                [&](auto&&... args) {
                    println(output_file_, type.color, "  ", left(get_local_time(), 36),
                            left(location, 63), left(std::string_view(type), 13),
                            std::forward<Args>(args)..., color::end());
                },
                tuple_args);
    }
    // clang-format on

    template <typename Output> friend Logger<Output>& salt::logger() noexcept;

private:
    constexpr Logger() noexcept : output_file_("log.txt", fast_io::open_mode::app) {
        using namespace fast_io::mnp;
        SALT_PRINT_LOGGER_HEADER_TO(output_file_)
    }
    constexpr ~Logger() = default;

    fast_io::obuf_file output_file_;
};

template <> struct [[maybe_unused]] Logger<Console> final {
    constexpr Logger(Logger const&)            = delete;
    constexpr Logger(Logger&&)                 = delete;
    constexpr Logger& operator=(Logger const&) = delete;
    constexpr Logger& operator=(Logger&&)      = delete;

    // clang-format off
    template <log_type Type, typename... Args>
    constexpr void log(Type                  type,
                       std::tuple<Args&&...> tuple_args,
                       source_location       location) noexcept {
        using namespace fast_io::mnp;
        std::apply(
                [&](auto&&... args) {
                    println(fast_io::out(), type.color, left(get_local_time(), 34), location, " ",
                            std::string_view(type), " ", std::forward<Args>(args)..., color::end());
                },
                tuple_args);
    }
    // clang-format on

    template <typename Output> friend Logger<Output>& salt::logger() noexcept;

private:
    constexpr Logger() noexcept = default;
    constexpr ~Logger()         = default;
};

#undef SALT_PRINT_LOGGER_HEADER_TO_FILE

} // namespace detail
} // namespace salt