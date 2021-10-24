find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

salt_win32_app(playground
               SOURCES "main.cpp"
               LINK    glad::glad
                       glfw::glfw
                       imgui::imgui)

# code: language="CMake" insertSpaces=true tabSize=4