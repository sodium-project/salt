if(TARGET glad::glad)
    return()
endif()
if(SALT_TARGET_OS STREQUAL "Windows")
    add_library(glad::glad INTERFACE IMPORTED)
    target_link_libraries(glad::glad
                          INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/lib/glad.lib")
    target_include_directories(glad::glad
                          INTERFACE "${CMAKE_BINARY_DIR}/output/libs/glad/include")
endif()