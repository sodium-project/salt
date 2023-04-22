if(TARGET glfw::glfw)
    return()
endif()

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(glfw::glfw INTERFACE IMPORTED)
    if(SALT_TARGET_OS STREQUAL "Windows")
        target_link_libraries(glfw::glfw
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glfw/lib/glfw3.lib")
    elseif(SALT_TARGET_OS STREQUAL "MacOSX")
        target_link_libraries(glfw::glfw INTERFACE "-framework Cocoa" "-framework IOKit")
        target_link_libraries(glfw::glfw
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glfw/lib/libglfw3.a")
    elseif(SALT_TARGET_OS STREQUAL "Linux")
        target_link_libraries(glfw::glfw
                            INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glfw/lib/libglfw3.a")
    endif()
    target_link_libraries(glfw::glfw INTERFACE ${GLFW_LIBRARIES})
    target_include_directories(glfw::glfw INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glfw/include")
endif()

# code: language="CMake" insertSpaces=true tabSize=4