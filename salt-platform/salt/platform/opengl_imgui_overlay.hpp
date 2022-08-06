#pragma once
#include <salt/platform/opengl_window.hpp>

namespace salt {

struct [[nodiscard]] Opengl_imgui_overlay final {

    Opengl_imgui_overlay(Opengl_window const& window) noexcept;
    ~Opengl_imgui_overlay();

    void render() const noexcept;
};

} // namespace salt