if(TARGET glfw::glfw)
    return()
endif()

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(glfw::glfw INTERFACE IMPORTED)
    if(SALT_TARGET_OS STREQUAL "Windows")
        target_link_libraries(glfw::glfw
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glfw/lib/glfw3.lib")
    endif()
    target_link_libraries(glfw::glfw INTERFACE ${GLFW_LIBRARIES})
    target_include_directories(glfw::glfw INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glfw/include")
endif()

# code: language="CMake" insertSpaces=true tabSize=4