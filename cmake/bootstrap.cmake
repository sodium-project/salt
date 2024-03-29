if($CACHE{SALT_BOOTSTRAP_DONE})
    return()
endif()

find_package(Git REQUIRED)

########################################################################################################################
# Update submodules.
########################################################################################################################

message(" ==============================================================================\n"
        " Updating Git submodules, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT)
if(NOT COMMAND_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to update Git submodules.")
else()
    message(STATUS "Git submodules are already up-to-date.")
endif()

########################################################################################################################
# CMake arguments that are used to configure thirdparty libraries.
########################################################################################################################

set(SALT_CMAKE_ARGUMENTS
    -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}

    -DCMAKE_CXX_STANDARD=23

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}

    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    -DCMAKE_C_LINKER_FLAGS=${CMAKE_C_LINKER_FLAGS}
    -DCMAKE_CXX_LINKER_FLAGS=${CMAKE_CXX_LINKER_FLAGS}

    -DCMAKE_C_FLAGS_${SALT_BUILD_TYPE}=${CMAKE_C_FLAGS_${SALT_BUILD_TYPE}}
    -DCMAKE_CXX_FLAGS_${SALT_BUILD_TYPE}=${CMAKE_CXX_FLAGS_${SALT_BUILD_TYPE}}
)

if(SALT_TARGET_OS STREQUAL "MacOSX")
    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}")
    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}")

    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OBJC_COMPILER=${CMAKE_OBJC_COMPILER}")
    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OBJCXX_COMPILER=${CMAKE_OBJCXX_COMPILER}")

    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OBJC_FLAGS=${CMAKE_OBJC_FLAGS}")
    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OBJCXX_FLAGS=${CMAKE_OBJCXX_FLAGS}")

    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OBJC_LINKER_FLAGS=${CMAKE_OBJC_LINKER_FLAGS}")
    list(APPEND SALT_CMAKE_ARGUMENTS "-DCMAKE_OBJCXX_LINKER_FLAGS=${CMAKE_OBJCXX_LINKER_FLAGS}")
endif()

########################################################################################################################
# Configure, build, and install Catch2.
########################################################################################################################

string(CONCAT SALT_CATCH2_CONFIG_CONSOLE_WIDTH "${CMAKE_CXX_FLAGS} -DCATCH_CONFIG_CONSOLE_WIDTH=300")

message(" ==============================================================================\n"
        " Configuring Catch2, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/Catch2
                        -S${CMAKE_SOURCE_DIR}/libs/Catch2
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/Catch2
                        -DCATCH_BUILD_STATIC_LIBRARY=ON
                        -DCATCH_BUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
                        -DCATCH_INSTALL_HELPERS=OFF
                        ${SALT_CMAKE_ARGUMENTS}
                        -DCMAKE_CXX_FLAGS=${SALT_CATCH2_CONFIG_CONSOLE_WIDTH}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure Catch2.")
endif()

message(" ==============================================================================\n"
        " Building and installing Catch2, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/libs/Catch2
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install Catch2.")
endif()

########################################################################################################################
# Configure, build, and install fast_io.
########################################################################################################################

message(" ==============================================================================\n"
        " Configuring fast_io, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/fast_io
                        -S${CMAKE_SOURCE_DIR}/libs/fast_io
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/fast_io
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
                        -DCMAKE_MSVC_RUNTIME_LIBRARY=${SALT_MSVC_RUNTIME_LIBRARY}
                        ${SALT_CMAKE_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure fast_io.")
endif()

message(" ==============================================================================\n"
        " Building and installing fast_io, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/libs/fast_io
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install fast_io.")
endif()

########################################################################################################################
# Configure, build, and install GLFW.
########################################################################################################################

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    message(" ==============================================================================\n"
            " Configuring GLFW, please wait...\n"
            " ==============================================================================")
    execute_process(COMMAND ${CMAKE_COMMAND}
                            -B${CMAKE_BINARY_DIR}/libs/glfw
                            -S${CMAKE_SOURCE_DIR}/libs/glfw
                            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/glfw
                            -DENKITS_BUILD_EXAMPLES=OFF
                            -DGLFW_BUILD_EXAMPLES=OFF
                            -DGLFW_BUILD_TESTS=OFF
                            -DGLFW_BUILD_DOCS=OFF
                            -DGLFW_BUILD_INSTALL=OFF
                            ${SALT_CMAKE_ARGUMENTS}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE COMMAND_RESULT
                    COMMAND_ECHO STDOUT)
    if(NOT COMMAND_RESULT STREQUAL "0")
        message(FATAL_ERROR "Failed to configure GLFW.")
    endif()

    message(" ==============================================================================\n"
            " Building and installing GLFW, please wait...\n"
            " ==============================================================================")
    execute_process(COMMAND ${CMAKE_COMMAND}
                            --build ${CMAKE_BINARY_DIR}/libs/glfw
                            --target install
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE COMMAND_RESULT
                    COMMAND_ECHO STDOUT)
    if(NOT COMMAND_RESULT STREQUAL "0")
        message(FATAL_ERROR "Failed to install GLFW.")
    endif()
endif()

########################################################################################################################
# Configure, build, and install glad.
########################################################################################################################

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    message(" ==============================================================================\n"
            " Configuring glad, please wait...\n"
            " ==============================================================================")
    execute_process(COMMAND ${CMAKE_COMMAND}
                            -B${CMAKE_BINARY_DIR}/libs/glad
                            -S${CMAKE_SOURCE_DIR}/libs/glad
                            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/glad
                            -DGLAD_PROFILE=core
                            -DGLAD_API="gl=${SALT_OPENGL_VERSION_MAJOR}.${SALT_OPENGL_VERSION_MINOR}" # API type/version pairs, like "gl=3.2,gles=3.2", no version means latest
                            -DGLAD_INSTALL=ON
                            ${SALT_CMAKE_ARGUMENTS}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE COMMAND_RESULT
                    COMMAND_ECHO STDOUT)
    if(NOT COMMAND_RESULT STREQUAL "0")
        message(FATAL_ERROR "Failed to configure glad.")
    endif()

    message(" ==============================================================================\n"
            " Building and installing glad, please wait...\n"
            " ==============================================================================")
    execute_process(COMMAND ${CMAKE_COMMAND}
                            --build ${CMAKE_BINARY_DIR}/libs/glad
                            --target install
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE COMMAND_RESULT
                    COMMAND_ECHO STDOUT)
    if(NOT COMMAND_RESULT STREQUAL "0")
        message(FATAL_ERROR "Failed to install glad.")
    endif()
endif()

########################################################################################################################
# Configure, build, and install Dear ImGui library.
########################################################################################################################

message(" ==============================================================================\n"
        " Generating a project for Dear ImGui, please wait...\n"
        " ==============================================================================")
file(WRITE "${CMAKE_BINARY_DIR}/generated/libs/imgui/CMakeLists.txt"
[[cmake_minimum_required(VERSION 3.20)
project(imgui CXX)

add_library(imgui STATIC)
target_include_directories(imgui PRIVATE "${IMGUI_ROOT}")
target_sources(imgui PRIVATE
               "${IMGUI_ROOT}/imgui.cpp"
               "${IMGUI_ROOT}/imgui_demo.cpp"
               "${IMGUI_ROOT}/imgui_draw.cpp"
               "${IMGUI_ROOT}/imgui_tables.cpp"
               "${IMGUI_ROOT}/imgui_widgets.cpp"
               "${IMGUI_ROOT}/misc/cpp/imgui_stdlib.cpp")
set(IMGUI_PUBLIC_HEADERS
    "${IMGUI_ROOT}/imgui.h"
    "${IMGUI_ROOT}/misc/cpp/imgui_stdlib.h")
set(IMGUI_PRIVATE_HEADERS
    "${IMGUI_ROOT}/imconfig.h"
    "${IMGUI_ROOT}/imstb_rectpack.h"
    "${IMGUI_ROOT}/imstb_textedit.h"
    "${IMGUI_ROOT}/imstb_truetype.h")
set_target_properties(imgui PROPERTIES
                      PUBLIC_HEADER  "${IMGUI_PUBLIC_HEADERS}"
                      PRIVATE_HEADER "${IMGUI_PRIVATE_HEADERS}")
install(TARGETS imgui)
]])

# This target is OpenGL-Windows-specific.
file(APPEND "${CMAKE_BINARY_DIR}/generated/libs/imgui/CMakeLists.txt"
[[if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(imgui_opengl STATIC)
    target_include_directories(imgui_opengl PRIVATE "${IMGUI_ROOT}" "${GLFW_ROOT}/include")
    target_sources(imgui_opengl PRIVATE
                   "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
                   "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp")
    set(IMGUI_OPENGL_PRIVATE_HEADERS
        "${IMGUI_ROOT}/backends/imgui_impl_glfw.h"
        "${IMGUI_ROOT}/backends/imgui_impl_opengl3.h")
    set_target_properties(imgui_opengl PROPERTIES
                          PRIVATE_HEADER "${IMGUI_OPENGL_PRIVATE_HEADERS}")
    install(TARGETS imgui_opengl)
endif()
]])

# This target is Metal-MacOSX-specific.
file(APPEND "${CMAKE_BINARY_DIR}/generated/libs/imgui/CMakeLists.txt"
[[if(SALT_TARGET_GRAPHICS STREQUAL "Metal")
    enable_language(OBJCXX)

    add_library(imgui_metal STATIC)
    target_include_directories(imgui_metal PRIVATE "${IMGUI_ROOT}")
    target_compile_options(imgui_metal PRIVATE -fobjc-arc)
    target_sources(imgui_metal PRIVATE
                   "${IMGUI_ROOT}/backends/imgui_impl_metal.mm"
                   "${IMGUI_ROOT}/backends/imgui_impl_osx.mm")
    set(IMGUI_METAL_PRIVATE_HEADERS
        "${IMGUI_ROOT}/backends/imgui_impl_metal.h"
        "${IMGUI_ROOT}/backends/imgui_impl_osx.h")
    set_target_properties(imgui_metal PROPERTIES
                          PRIVATE_HEADER "${IMGUI_METAL_PRIVATE_HEADERS}")
    target_link_libraries(imgui_metal PRIVATE "-framework Metal" "-framework AppKit")
    install(TARGETS imgui_metal)
endif()
]])

message(" ==============================================================================\n"
        " Configuring Dear ImGui, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/imgui
                        -S${CMAKE_BINARY_DIR}/generated/libs/imgui
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/imgui
                        -DIMGUI_ROOT=${CMAKE_SOURCE_DIR}/libs/imgui
                        -DGLFW_ROOT=${CMAKE_SOURCE_DIR}/libs/glfw
                        -DSALT_TARGET_GRAPHICS=${SALT_TARGET_GRAPHICS}
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
                        -DCMAKE_MSVC_RUNTIME_LIBRARY=${SALT_MSVC_RUNTIME_LIBRARY}
                        ${SALT_CMAKE_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure Dear ImGui.")
endif()

message(" ==============================================================================\n"
        " Building and installing Dear ImGui, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/libs/imgui
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install Dear ImGui.")
endif()


########################################################################################################################
# Configure, build, and install entt.
########################################################################################################################

message(" ==============================================================================\n"
        " Configuring entt, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/entt
                        -S${CMAKE_SOURCE_DIR}/libs/entt
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/entt
                        ${SALT_CMAKE_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure entt.")
endif()

message(" ==============================================================================\n"
        " Building and installing entt, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/libs/entt
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install entt.")
endif()

########################################################################################################################
# Update CMakeCache.txt
########################################################################################################################

set(SALT_BOOTSTRAP_DONE TRUE CACHE BOOL "Whether the bootstrapping has been done." FORCE)

# code: language='CMake' insertSpaces=true tabSize=4