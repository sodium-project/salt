find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

salt_executable(playground
    COMMON
        LINK
            salt::core
            imgui::imgui
    MACOSX
        SOURCE
            "playground.mm"
        LINK
            "-framework MetalKit"
    WINDOWS
        SOURCE
            "playground.cpp"
    LINUX
        SOURCE
            "playground.cpp"
)

# code: language="CMake" insertSpaces=true tabSize=4