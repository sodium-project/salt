#pragma once

#include <string_view>
#include <variant>

// TODO:
//  add macro checks
#include <salt/core/platform/win64_window.hpp>

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

    std::variant<std::monostate, Win64_window> platform_window_;
};

} // namespace salt