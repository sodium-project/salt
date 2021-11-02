find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

salt_common_app(playground_win
                SOURCES "playground.cpp"
                LINK    salt::core
                        glad::glad
                        glfw::glfw
                        imgui::imgui)

# code: language="CMake" insertSpaces=true tabSize=4