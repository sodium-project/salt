#pragma once
#include <salt/core/window.hpp>

namespace salt {

#if defined(SALT_TARGET_OPENGL)
using Imgui_overlay = is_overlay<Glfw_window, Glfw_opengl_imgui_overlay>;
#elif defined(SALT_TARGET_METAL)
using Imgui_overlay = is_overlay<Metal_window, Metal_imgui_overlay>;
#endif

} // namespace salt