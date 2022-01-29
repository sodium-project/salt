find_package(glfw REQUIRED)
find_package(glad REQUIRED)

salt_static_library(core
                    COMMON_SOURCE "salt/core/engine.cpp"
                                  "salt/core/entry_point.cpp"
                                  "salt/core/platform/win64_window.cpp"
                    PUBLIC_LINK   glad::glad
                                  glfw::glfw
                                  salt::events
                                  salt::utils)

# code: language="CMake" insertSpaces=true tabSize=4