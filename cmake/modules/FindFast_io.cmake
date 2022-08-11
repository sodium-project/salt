if(TARGET fast_io::fast_io)
    return()
endif()
add_library(fast_io::fast_io INTERFACE IMPORTED)

target_include_directories(fast_io::fast_io
                           INTERFACE "${CMAKE_BINARY_DIR}/output/libs/fast_io/include")

# code: language="CMake" insertSpaces=true tabSize=4