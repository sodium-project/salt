#pragma once

#include <string_view>

// TODO:
//  add macro checks
#include <salt/core/window.hpp>

#include <salt/platform.hpp>

namespace salt {

struct [[nodiscard]] Command_line_args final {
    std::size_t        count  = 0;
    char const* const* vector = nullptr;

    std::string_view operator[](std::size_t const i) const noexcept;
};

struct [[nodiscard]] Application final {
    using Fn = void (*)();

    Application(Command_line_args args = Command_line_args{}) noexcept;

    void run(Fn fn) const noexcept;

private:
    Platform_window window_;
    Imgui_overlay   imgui_overlay_;
};

} // namespace salt