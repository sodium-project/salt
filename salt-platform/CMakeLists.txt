find_package(glfw REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

salt_static_library(platform
    COMMON
        SOURCE
            "salt/platform/glfw_window.cpp"
            "salt/platform/glfw_opengl_imgui_overlay.cpp"
        LINK
            glad::glad
            glfw::glfw
            imgui::imgui
            salt::events
            salt::foundation)

# code: language="CMake" insertSpaces=true tabSize=4