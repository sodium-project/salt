#pragma once
#include <salt/meta.hpp>
#include <salt/logging/detail/source_location.hpp>

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

struct [[nodiscard]] print_color final {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    static constexpr std::string_view end() noexcept {
        return "\033[0;00m";
    }
};

constexpr std::size_t print_reserve_size(fast_io::io_reserve_type_t<char, print_color>) {
    using namespace fast_io;
    constexpr auto reserve_size = print_reserve_size(io_reserve_type<char, std::uint8_t>);
    constexpr auto total_size   = (reserve_size * 3 + 10);
    return total_size;
}

template <meta::contiguous_iterator Iter>
constexpr Iter
print_reserve_define(fast_io::io_reserve_type_t<meta::iter_value_t<Iter>, print_color>, Iter it,
                     print_color color) {
    using namespace fast_io;
    *(it = details::non_overlapped_copy_n("\033[38;2", sizeof "\033[38;2" - 1, it))  = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.r)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.g)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.b)) = 'm';
    return ++it;
}
static_assert(fast_io::reserve_printable<char, print_color>);

struct [[nodiscard]] trace_tag final {
    print_color color;
    constexpr operator std::string_view() const noexcept {
        return "[TRACE]";
    }
};
struct [[nodiscard]] debug_tag final {
    print_color color;
    constexpr operator std::string_view() const noexcept {
        return "[DEBUG]";
    }
};
struct [[nodiscard]] warning_tag final {
    print_color color;
    constexpr operator std::string_view() const noexcept {
        return "[WARNING]";
    }
};
struct [[nodiscard]] error_tag final {
    print_color color;
    constexpr operator std::string_view() const noexcept {
        return "[ERROR]";
    }
};

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

template <typename Type>
concept log_type = 
    meta::is_class_v<Type>                       and
    meta::default_initializable<Type>            and
    meta::convertible_to<Type, std::string_view> and
    requires(Type type) {
        { meta::prvalue(type.color) } -> log_color;
        // Requires that constructing a Type from `color` is a valid expression
        Type{type.color};
    };
// clang-format on

namespace detail {
// clang-format off
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
    io::println(output_file, mnp::left("+", 36, '-'), mnp::left("+", 63, '-'),                     \
                mnp::left("+", 13, '-'), mnp::left("+", 32, '-'));                                 \
    io::println(output_file, "| ", mnp::left("Date / Time", 33), " | ", mnp::left("Location", 60), \
                " | ", mnp::left("Level", 10), " | Message");                                      \
    io::println(output_file, mnp::left("+", 36, '-'), mnp::left("+", 63, '-'),                     \
                mnp::left("+", 13, '-'), mnp::left("+", 32, '-'));

// clang-format off
template <>
struct [[maybe_unused]] dummy_logger<file_tag> final {
    constexpr dummy_logger(dummy_logger const&)            = delete;
    constexpr dummy_logger(dummy_logger&&)                 = delete;
    constexpr dummy_logger& operator=(dummy_logger const&) = delete;
    constexpr dummy_logger& operator=(dummy_logger&&)      = delete;

    template <log_type Type, typename... Args>
    constexpr void log(Type                   type,
                       meta::tuple<Args&&...> tuple_args,
                       source_location        location) noexcept {
        using namespace fast_io;

        [[maybe_unused]] fast_io::io_flush_guard guard{output_file_};
        meta::apply(
                [&](auto&&... args) {
                    io::println(output_file_, type.color, "  ", mnp::left(get_local_time(), 36),
                                mnp::left(location, 63), mnp::left(std::string_view(type), 13),
                                meta::forward<Args>(args)..., print_color::end());
                },
                tuple_args);
    }

    template <typename Output>
    friend dummy_logger<Output>& salt::log::logger() noexcept;

private:
    constexpr dummy_logger() noexcept : output_file_("log.txt", fast_io::open_mode::app) {
        using namespace fast_io;
        SALT_PRINT_LOGGER_HEADER_TO(output_file_)
    }
    constexpr ~dummy_logger() = default;

    fast_io::obuf_file output_file_;
};

template <>
struct [[maybe_unused]] dummy_logger<console_tag> final {
    constexpr dummy_logger(dummy_logger const&)            = delete;
    constexpr dummy_logger(dummy_logger&&)                 = delete;
    constexpr dummy_logger& operator=(dummy_logger const&) = delete;
    constexpr dummy_logger& operator=(dummy_logger&&)      = delete;

    template <log_type Type, typename... Args>
    constexpr void log(Type                   type,
                       meta::tuple<Args&&...> tuple_args,
                       source_location        location) noexcept {
        using namespace fast_io;
        meta::apply(
                [&](auto&&... args) {
                    io::println(fast_io::out(), type.color, mnp::left(get_local_time(), 34),
                                location, " ", std::string_view(type), " ",
                                meta::forward<Args>(args)..., print_color::end());
                },
                tuple_args);
    }

    template <typename Output>
    friend dummy_logger<Output>& salt::log::logger() noexcept;

private:
    constexpr dummy_logger() noexcept = default;
    constexpr ~dummy_logger()         = default;
};
// clang-format on

#undef SALT_PRINT_LOGGER_HEADER_TO_FILE

} // namespace detail
} // namespace salt::log