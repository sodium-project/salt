if(TARGET Catch2::Catch2)
    return()
endif()
add_library(Catch2::Catch2 INTERFACE IMPORTED)

target_include_directories(Catch2::Catch2
                           INTERFACE "${CMAKE_BINARY_DIR}/output/libs/Catch2/include")

# code: language="CMake" insertSpaces=true tabSize=4