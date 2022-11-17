#pragma once
#include <string>

namespace salt::detail {

constexpr bool is_path_sep(char c) noexcept {
    return c == '/' || c == '\\';
}

constexpr char const* strip_path(char const* path) noexcept {
    auto last_name = path;
    for (auto p = path; *p; ++p) {
        if (is_path_sep(*p) && *(p + 1))
            last_name = p + 1;
    }
    return last_name;
}

struct [[nodiscard]] basename final {
    constexpr basename(char const* begin, char const* end) noexcept : begin_{begin}, end_{end} {}

    std::string as_string() const {
        return std::string(begin_, end_);
    }

    char const* const begin_;
    char const* const end_;
};

inline std::string to_string(basename const& basename) {
    return basename.as_string();
}

constexpr char const* last_dot_of(char const* p) {
    char const* last_dot = nullptr;
    for (; *p; ++p) {
        if (*p == '.')
            last_dot = p;
    }
    return last_dot ? last_dot : p;
}

} // namespace salt::detail