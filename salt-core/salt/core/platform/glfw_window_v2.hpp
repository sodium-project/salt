#pragma once
#include <salt/math.hpp>

using GLFWwindow = struct GLFWwindow;

namespace salt::detail {

struct [[nodiscard]] GLFW_window final {
    ::GLFWwindow*  window;
    salt::Size     size     = {};
    salt::Position position = {};

    ~GLFW_window();
};

} // namespace salt::detail