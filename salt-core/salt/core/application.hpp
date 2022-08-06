#pragma once

#include <string_view>

#include <salt/core/overlay.hpp>

namespace salt {

struct [[nodiscard]] Command_line_args final {
    std::size_t argc = 0;
    char**      argv = nullptr;

    std::string_view operator[](std::size_t idx) const noexcept {
        // TODO:
        //  Add assertion check
        return argv[idx];
    }
};

struct [[nodiscard]] Application final {
    using Fn = void (*)();

    Application(Command_line_args args = Command_line_args{}) noexcept;

    void run(Fn fn) const noexcept;

private:
    void on(Window_close_event& event) noexcept;
    void on(Window_resize_event& event) noexcept;

    bool          running_   = true;
    bool          minimized_ = false;
    Window        window_;
    Imgui_overlay overlay_;
};

} // namespace salt