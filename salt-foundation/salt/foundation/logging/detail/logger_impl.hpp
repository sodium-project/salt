#pragma once
#include <salt/meta.hpp>

#include <salt/foundation/io.hpp>
#include <salt/foundation/logging/detail/source_location.hpp>

#include <cstddef>
#include <string_view>

// NOTE:
//  This header is such a mess, it will need to be revisited in the future and probably refactored.
//  Maybe add multithreading support as well. But at this stage, let it be as it is.
// TODO:
//  Revisit it.
namespace salt::log {

struct [[nodiscard]] file_tag    final {};
struct [[nodiscard]] console_tag final {};

struct [[nodiscard]] ansi_color final {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255; // << Unused, exists only for alignment.

    struct [[nodiscard]] escape_code final {
        static constexpr std::string_view begin() noexcept {
            return "\033[38;2;";
        }
        static constexpr std::string_view end() noexcept {
            return "\033[0;00m";
        }
    };
};

constexpr auto print_reserve_size(fast_io::io_reserve_type_t<char, ansi_color>) noexcept {
    using namespace fast_io;
    constexpr auto reserve_size = print_reserve_size(io_reserve_type<char, std::uint8_t>);
    constexpr auto total_size   = reserve_size * 3;
    return total_size;
}

constexpr auto print_reserve_define(fast_io::io_reserve_type_t<char, ansi_color>, char* it,
                                    ansi_color color) noexcept {
    using namespace fast_io;
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.r)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.g)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.b)) = 'm';
    return ++it;
}
static_assert(fast_io::reserve_printable<char, ansi_color>);

template <typename T, typename... Args>
constexpr void println(ansi_color color, T&& device, Args&&... args) noexcept {
    io::println(meta::forward<T>(device), ansi_color::escape_code::begin(), color,
                meta::forward<Args>(args)..., ansi_color::escape_code::end());
}

namespace detail {

// clang-format off
template <typename Color>
concept log_color = 
    meta::is_class_v<Color>            and
    meta::default_initializable<Color> and
    requires(Color color) {
        { meta::prvalue(color.r) } -> meta::same_as<std::uint8_t>;
        { meta::prvalue(color.g) } -> meta::same_as<std::uint8_t>;
        { meta::prvalue(color.b) } -> meta::same_as<std::uint8_t>;
        { meta::prvalue(color.a) } -> meta::same_as<std::uint8_t>;
        requires sizeof(std::uint8_t) * 4 == sizeof(color);
        // Requires that constructing a Color from `r, g, b, a` is a valid expression.
        Color{color.r, color.g, color.b, color.a};
    };

template <typename Level>
concept log_level = 
    meta::is_class_v<Level>                       and
    meta::default_initializable<Level>            and
    requires(Level level) {
        { meta::prvalue(Level::color)  } -> log_color;
        { meta::prvalue(Level::status) } -> meta::same_as<std::string_view>;
    };

template <typename Output>
struct [[maybe_unused]] dummy_logger;
// clang-format on

} // namespace detail

// clang-format off
template <typename Output>
detail::dummy_logger<Output>& logger() noexcept;
// clang-format on

namespace detail {

inline auto get_local_time() noexcept {
    [[maybe_unused]] static bool once = ([] { fast_io::posix_tzset(); }(), true);
    return local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime));
}

#define SALT_PRINT_LOGGER_HEADER_TO(output_file)                                                   \
    io::println(output_file, left("+", 36, '-'), left("+", 63, '-'), left("+", 13, '-'),           \
                left("+", 32, '-'));                                                               \
    io::println(output_file, "| ", left("Date / Time", 33), " | ", left("Location", 60), " | ",    \
                left("Level", 10), " | Message");                                                  \
    io::println(output_file, left("+", 36, '-'), left("+", 63, '-'), left("+", 13, '-'),           \
                left("+", 32, '-'));

// clang-format off
template <>
struct [[maybe_unused]] dummy_logger<file_tag> final {
    constexpr dummy_logger(dummy_logger const&)            = delete;
    constexpr dummy_logger(dummy_logger&&)                 = delete;
    constexpr dummy_logger& operator=(dummy_logger const&) = delete;
    constexpr dummy_logger& operator=(dummy_logger&&)      = delete;

    template <detail::log_level Level, typename... Args>
    constexpr void log(Level, meta::tuple<Args&&...> tuple, source_location location) noexcept {
        using namespace io;
        [[maybe_unused]] io_flush_guard guard{output_file_};
        meta::apply(
                [&](auto&&... args) {
                    println(Level::color, output_file_, "  ", left(get_local_time(), 36),
                            left(location, 63), left(Level::status, 13),
                            meta::forward<Args>(args)...);
                },
                tuple);
    }

    template <typename Output>
    friend dummy_logger<Output>& salt::log::logger() noexcept;

private:
    constexpr dummy_logger() noexcept : output_file_{"log.ansi", io::open_mode::app} {
        using namespace io;
        SALT_PRINT_LOGGER_HEADER_TO(output_file_)
    }
    constexpr ~dummy_logger() = default;

    io::obuf_file output_file_;
};

template <>
struct [[maybe_unused]] dummy_logger<console_tag> final {
    constexpr dummy_logger(dummy_logger const&)            = delete;
    constexpr dummy_logger(dummy_logger&&)                 = delete;
    constexpr dummy_logger& operator=(dummy_logger const&) = delete;
    constexpr dummy_logger& operator=(dummy_logger&&)      = delete;

    template <detail::log_level Level, typename... Args>
    constexpr void log(Level, meta::tuple<Args&&...> tuple, source_location location) noexcept {
        using namespace io;
        meta::apply(
                [&](auto&&... args) {
                    println(Level::color, out(), left(get_local_time(), 34), location,
                            " ", Level::status, " ", meta::forward<Args>(args)...);
                },
                tuple);
    }

    template <typename Output>
    friend dummy_logger<Output>& salt::log::logger() noexcept;

private:
    constexpr dummy_logger() noexcept = default;
    constexpr ~dummy_logger()         = default;
};
// clang-format on

#undef SALT_PRINT_LOGGER_HEADER_TO

} // namespace detail
} // namespace salt::log