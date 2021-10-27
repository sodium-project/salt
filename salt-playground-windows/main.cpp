#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <cstdio>

#include <salt/core.hpp>

int main() {
    salt::Engine engine;
    engine.run();
}
