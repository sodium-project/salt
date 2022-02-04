find_package(glfw REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

salt_static_library(platform
                    COMMON_SOURCE "salt/platform/glfw_window.cpp"
                                  "salt/platform/macos_window.cpp"
                    PUBLIC_LINK   glad::glad
                                  glfw::glfw
                                  imgui::imgui
                                  salt::events
                                  salt::utils)

# code: language="CMake" insertSpaces=true tabSize=4