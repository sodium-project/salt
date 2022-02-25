find_package(entt REQUIRED)

salt_static_library(core
                    COMMON_SOURCE "salt/core/application.cpp"
                                  "salt/core/entry_point.cpp"
                    PUBLIC_LINK   entt::entt
                                  salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4