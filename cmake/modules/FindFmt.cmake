if(TARGET fmt::fmt)
    return()
endif()
add_library(fmt::fmt INTERFACE IMPORTED)
if(SALT_TARGET_OS STREQUAL "Windows")
    target_link_libraries(fmt::fmt INTERFACE "${CMAKE_BINARY_DIR}/output/libs/fmt/lib/fmt.lib")
elseif(SALT_TARGET_OS STREQUAL "MacOSX")
    target_link_libraries(fmt::fmt INTERFACE "${CMAKE_BINARY_DIR}/output/libs/fmt/lib/libfmt.a")
endif()
target_include_directories(fmt::fmt INTERFACE "${CMAKE_BINARY_DIR}/output/libs/fmt/include")

# code: language="CMake" insertSpaces=true tabSize=4