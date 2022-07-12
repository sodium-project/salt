find_package(entt REQUIRED)

salt_static_library(core
    WINDOWS
        SOURCE
            "salt/core/platform/glfw_window_v2.cpp"
    COMMON
        SOURCE
            "salt/core/application.cpp"
            "salt/core/entry_point.cpp"
        LINK
            entt::entt
            salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4