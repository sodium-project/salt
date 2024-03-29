if(TARGET imgui::imgui)
    return()
endif()
add_library(imgui::imgui INTERFACE IMPORTED)
if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    if(SALT_TARGET_OS STREQUAL "Windows")
        target_link_libraries(imgui::imgui
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/imgui/lib/imgui.lib"
                                        "${CMAKE_BINARY_DIR}/output/libs/imgui/lib/imgui_opengl.lib")
    elseif(SALT_TARGET_OS STREQUAL "MacOSX" OR SALT_TARGET_OS STREQUAL "Linux")
        target_link_libraries(imgui::imgui
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/imgui/lib/libimgui.a"
                                        "${CMAKE_BINARY_DIR}/output/libs/imgui/lib/libimgui_opengl.a")
    endif()
elseif(SALT_TARGET_GRAPHICS STREQUAL "Metal")
    if(SALT_TARGET_OS STREQUAL "MacOSX")
        target_compile_options(imgui::imgui INTERFACE "-fobjc-arc")
        target_link_libraries(imgui::imgui INTERFACE "-framework Metal" "-framework AppKit")
        target_link_libraries(imgui::imgui 
                              INTERFACE "${CMAKE_BINARY_DIR}/output/libs/imgui/lib/libimgui.a"
                                        "${CMAKE_BINARY_DIR}/output/libs/imgui/lib/libimgui_metal.a")
    endif()
endif()
target_include_directories(imgui::imgui INTERFACE "${CMAKE_BINARY_DIR}/output/libs/imgui/include")

# code: language="CMake" insertSpaces=true tabSize=4