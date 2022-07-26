#pragma once

#include <salt/platform/glfw_window.hpp>

namespace salt {

struct [[nodiscard]] Glfw_opengl_imgui_overlay final {

    Glfw_opengl_imgui_overlay() noexcept;

    ~Glfw_opengl_imgui_overlay();

    void attach(Glfw_window const& window) const noexcept;
    void detach() const noexcept;

    void render() const noexcept;
};

} // namespace salt