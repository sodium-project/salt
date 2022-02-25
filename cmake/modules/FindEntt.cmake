if(TARGET entt::entt)
    return()
endif()
add_library(entt::entt INTERFACE IMPORTED)

target_include_directories(entt::entt
                           INTERFACE "${CMAKE_BINARY_DIR}/output/libs/entt/include")

# code: language="CMake" insertSpaces=true tabSize=4