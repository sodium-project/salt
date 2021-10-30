find_package(fmt REQUIRED)

salt_static_library(core
                    COMMON_SOURCE "salt/core/engine.cpp"
                    PUBLIC_LINK   fmt::fmt)

# code: language="CMake" insertSpaces=true tabSize=4