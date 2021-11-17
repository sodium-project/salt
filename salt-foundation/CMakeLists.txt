find_package(fmt REQUIRED)

set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

salt_interface_library(foundation
                       COMMON_TEST "salt/foundation/utils-test.cpp"
                                   "salt/foundation/detail/source_location-test.cpp"
                       PUBLIC_LINK fmt::fmt
                                   salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4