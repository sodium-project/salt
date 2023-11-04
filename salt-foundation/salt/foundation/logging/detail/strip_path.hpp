#pragma once

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

constexpr char const* last_dot_of(char const* p) {
    char const* last_dot = nullptr;
    for (; *p; ++p) {
        if (*p == '.')
            last_dot = p;
    }
    return last_dot ? last_dot : p;
}

} // namespace salt::detail