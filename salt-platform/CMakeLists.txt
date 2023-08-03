find_package(glfw REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

salt_static_library(platform
    COMMON
        SOURCE
            "salt/platform/dummy.cpp"
        LINK
            salt::config)

# code: language="CMake" insertSpaces=true tabSize=4