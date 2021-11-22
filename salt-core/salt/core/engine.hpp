#pragma once

#include <string_view>

namespace salt {

struct [[nodiscard]] Command_line_args final {
    std::size_t        count  = 0;
    char const* const* vector = nullptr;

    std::string_view operator[](std::size_t const i) const noexcept;
};

struct [[nodiscard]] Engine final {
    using Fn = void (*)();

    Engine(Command_line_args args = Command_line_args{}) noexcept;

    void run(Fn fn) const noexcept;
};

} // namespace salt