if(TARGET glad::glad)
    return()
endif()

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(glad::glad INTERFACE IMPORTED)
    if(SALT_TARGET_OS STREQUAL "Windows")
        target_link_libraries(glad::glad
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/lib/glad.lib")
    elseif(SALT_TARGET_OS STREQUAL "MacOSX" OR SALT_TARGET_OS STREQUAL "Linux")
        target_link_libraries(glad::glad
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/lib/libglad.a")
    endif()
    target_include_directories(glad::glad
                               INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/include")
endif()

# code: language="CMake" insertSpaces=true tabSize=4