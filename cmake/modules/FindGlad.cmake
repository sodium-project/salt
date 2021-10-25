if(TARGET glad::glad)
    return()
endif()

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(glad::glad INTERFACE IMPORTED)
    if(SALT_TARGET_OS STREQUAL "Windows")
        target_link_libraries(glad::glad
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/lib/glad.lib")
    endif()
    target_include_directories(glad::glad
                               INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/include")
endif()

# code: language="CMake" insertSpaces=true tabSize=4