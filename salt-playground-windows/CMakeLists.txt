find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

salt_common_app(playground_win
                SOURCES "playground.cpp"
                LINK    salt::core
                        glad::glad
                        glfw::glfw
                        imgui::imgui)

# code: language="CMake" insertSpaces=true tabSize=4