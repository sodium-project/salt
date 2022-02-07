#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <string_view>

#include <salt/events.hpp>
#include <salt/meta.hpp>

namespace salt {

struct [[nodiscard]] Glfw_window final {

    Glfw_window(Size const& size, Position const& position = Position{}) noexcept;

    ~Glfw_window();

    Size size() const noexcept;

    void update() const noexcept;

    bool alive() const noexcept;

private:
    ::GLFWwindow* window_;

    std::string_view title_;
    Size             size_;
    Position         position_;

    Event_dispatcher dispatcher_;

    friend struct Glfw_opengl_imgui_overlay;
};

// TODO:
//  Add a macro check for the window implementation we need.
using Platform_window = Glfw_window;

inline windowable auto make_window(Size const size, Position const position) noexcept {
    return Glfw_window{size, position};
}

} // namespace salt