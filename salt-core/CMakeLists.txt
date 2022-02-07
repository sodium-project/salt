find_package(glfw REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

salt_static_library(core
                    COMMON_SOURCE "salt/core/application.cpp"
                                  "salt/core/entry_point.cpp"
                    PUBLIC_LINK   salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4