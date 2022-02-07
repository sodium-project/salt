find_package(glfw REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

salt_static_library(platform
                    COMMON_SOURCE "salt/platform/glfw_window.cpp"
                                  "salt/platform/glfw_opengl_imgui_overlay.cpp"
                    PRIVATE_LINK  glad::glad
                                  glfw::glfw
                                  imgui::imgui
                    PUBLIC_LINK   salt::events
                                  salt::utils)

# code: language="CMake" insertSpaces=true tabSize=4