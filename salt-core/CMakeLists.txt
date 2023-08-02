find_package(entt REQUIRED)

salt_static_library(core
    COMMON
        SOURCE
            "salt/core/application.cpp"
            "salt/core/entry_point.cpp"
        LINK
            salt::config)

# code: language="CMake" insertSpaces=true tabSize=4