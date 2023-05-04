find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

salt_executable(playground
    COMMON
        SOURCE
            "playground.cpp"
        LINK
            salt::core
            imgui::imgui
    MACOSX
        SOURCE
            "playground.mm"
        LINK
            salt::core
            imgui::imgui
            "-framework MetalKit"
)

# code: language="CMake" insertSpaces=true tabSize=4