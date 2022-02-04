// #include <imgui.h>
// #include <imgui_impl_glfw.h>
// #include <imgui_impl_opengl3.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <cstdio>
#include <salt/config.hpp>
#include <salt/core.hpp>
#include <salt/utils.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
// #define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include "imgui-demo.inl"
#endif

void salt::application_main() {
#ifdef RUN_IMGUI_DEMO
    show_imgui_demo();
#endif
}