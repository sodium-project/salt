find_package(glfw REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

salt_interface_library(platform
    COMMON
        LINK
            salt::config
            salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4