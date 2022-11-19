#pragma once

#include <string_view>

#include <salt/core/layer_stack.hpp>
#include <salt/core/overlay.hpp>

namespace salt {

struct [[nodiscard]] Command_line_args final {
    int  const         argc = 0;
    char const* const* argv = nullptr;

    std::string_view operator[](std::size_t idx) const noexcept {
        // TODO:
        //  Add assertion check
        return argv[idx];
    }
};

struct [[nodiscard]] Application final {

    Application(Command_line_args args = Command_line_args{}) noexcept;

    void run() noexcept;

    template <typename Layer> void push() {
        layer_stack_.push<Layer>();
    }

private:
    void on(Window_close_event& event) noexcept;
    void on(Window_resize_event& event) noexcept;

    bool          running_   = true;
    bool          minimized_ = false;
    Window        window_;
    Imgui_overlay overlay_;
    Layer_stack   layer_stack_;
};

} // namespace salt