#pragma once

#include <string_view>

#include <salt/events.hpp>
#include <salt/meta.hpp>

using GLFWwindow = struct GLFWwindow;

namespace salt {

struct [[nodiscard]] Glfw_window final {

    Glfw_window() noexcept;
    Glfw_window(Size size, Position position = Position{}) noexcept;

    ~Glfw_window();

    void update() const noexcept;
    bool alive() const noexcept;

    bool vsync() const noexcept;
    void vsync(bool flag) noexcept;

    Size     size() const noexcept;
    Position position() const noexcept;

    void on(Window_close_event& event) noexcept;
    void on(Window_resize_event& event) noexcept;
    void on(Key_pressed_event& event) noexcept;
    void on(Key_released_event& event) noexcept;
    void on(Mouse_pressed_event& event) noexcept;
    void on(Mouse_released_event& event) noexcept;

private:
    using event_bus = registered_events::apply<Event_bus>;

    ::GLFWwindow* window_;

    std::string title_;
    Size        size_;
    Position    position_;
    bool        vsync_;

    event_bus bus_;

    friend struct Glfw_opengl_imgui_overlay;
};

} // namespace salt