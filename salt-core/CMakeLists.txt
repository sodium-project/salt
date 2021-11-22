find_package(fmt REQUIRED)

salt_static_library(core
                    COMMON_SOURCE "salt/core/engine.cpp"
                                  "salt/core/entry_point.cpp"
                    PUBLIC_LINK   salt::foundation)

# code: language="CMake" insertSpaces=true tabSize=4