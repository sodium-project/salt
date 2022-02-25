#pragma once

#include <salt/platform/glfw_window.hpp>

namespace salt {

struct [[nodiscard]] Glfw_opengl_imgui_overlay final {

    Glfw_opengl_imgui_overlay() noexcept;

    ~Glfw_opengl_imgui_overlay();

    void attach(Platform_window const& window) const noexcept;
    void detach() const noexcept;

    void render() const noexcept;
};

// TODO:
//  Add a macro check for the window implementation we need.
using Imgui_overlay = is_overliable<Platform_window, Glfw_opengl_imgui_overlay>;

} // namespace salt