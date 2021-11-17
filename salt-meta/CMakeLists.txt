set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

salt_interface_library(meta)

# code: language="CMake" insertSpaces=true tabSize=4