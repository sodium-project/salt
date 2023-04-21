if(TARGET glm::glm)
    return()
endif()
add_library(glm::glm INTERFACE IMPORTED)
target_include_directories(glm::glm
                           INTERFACE "${CMAKE_SOURCE_DIR}/libs/glm")

# code: language="CMake" insertSpaces=true tabSize=4