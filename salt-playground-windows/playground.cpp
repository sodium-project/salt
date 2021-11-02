//#define _ITERATOR_DEBUG_LEVEL 2
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <Windows.h>
#include <cstdio>

#include <salt/core.hpp>

// NOTE(Andrii):
//  Get rid of this hack in the future.
#define RUN_IMGUI_DEMO (1)

#ifdef RUN_IMGUI_DEMO
#    include "imgui-demo.inl"
#endif

void salt::application_main() {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(handle, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    salt::trace("Hello from the playground!");
    salt::debug("Hello from the playground!");
    salt::info("Hello from the playground!");
    salt::warning("Hello from the playground!");
    salt::error("Hello from the playground!");
    salt::critical("Hello from the playground!");

#ifdef RUN_IMGUI_DEMO
    show_imgui_demo();
#endif
}