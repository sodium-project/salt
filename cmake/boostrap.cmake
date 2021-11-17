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
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}

    -DCMAKE_CXX_STANDARD=20

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}

    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    -DCMAKE_C_LINKER_FLAGS=${CMAKE_C_LINKER_FLAGS}
    -DCMAKE_CXX_LINKER_FLAGS=${CMAKE_CXX_LINKER_FLAGS}
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
message(" ==============================================================================\n"
        " Configuring Catch2, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/Catch2
                        -S${CMAKE_SOURCE_DIR}/libs/Catch2
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/Catch2
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
                        -DCATCH_BUILD_STATIC_LIBRARY=ON
                        -DCATCH_BUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
                        -DCATCH_INSTALL_HELPERS=OFF
                        ${SALT_CMAKE_ARGUMENTS}
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
# Configure, build, and install fmt.
########################################################################################################################

message(" ==============================================================================\n"
        " Configuring fmt, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/fmt
                        -S${CMAKE_SOURCE_DIR}/libs/fmt
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/fmt
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF # Workaround for [LNK4217] warning
                        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
                        -DFMT_DOC=OFF
                        -DFMT_TEST=OFF
                        -DFMT_DEBUG_POSTFIX:STRING=
                        ${SALT_CMAKE_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure fmt.")
endif()

message(" ==============================================================================\n"
        " Building and installing fmt, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/libs/fmt
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install fmt.")
endif()


########################################################################################################################
# Configure, build, and install GLFW.
########################################################################################################################

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL" AND SALT_TARGET_OS STREQUAL "Windows")
    message(" ==============================================================================\n"
            " Configuring GLFW, please wait...\n"
            " ==============================================================================")
    execute_process(COMMAND ${CMAKE_COMMAND}
                            -B${CMAKE_BINARY_DIR}/libs/glfw
                            -S${CMAKE_SOURCE_DIR}/libs/glfw
                            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/glfw
                            -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF # Workaround for [LNK4217] warning
                            -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug
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

if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL" AND SALT_TARGET_OS STREQUAL "Windows")
    message(" ==============================================================================\n"
            " Configuring glad, please wait...\n"
            " ==============================================================================")
    execute_process(COMMAND ${CMAKE_COMMAND}
                            -B${CMAKE_BINARY_DIR}/libs/glad
                            -S${CMAKE_SOURCE_DIR}/libs/glad
                            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/glad
                            -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF # Workaround for [LNK4217] warning
                            -DGLAD_PROFILE=core
                            -DGLAD_API="gl=4.6" # API type/version pairs, like "gl=3.2,gles=", no version means latest
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

if(MSVC AND NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_library(imgui STATIC)
target_include_directories(imgui PRIVATE "${IMGUI_ROOT}")
target_sources(imgui PRIVATE
               "${IMGUI_ROOT}/imgui.cpp"
               "${IMGUI_ROOT}/imgui_demo.cpp"
               "${IMGUI_ROOT}/imgui_draw.cpp"
               "${IMGUI_ROOT}/imgui_tables.cpp"
               "${IMGUI_ROOT}/imgui_widgets.cpp")
set(IMGUI_PRIVATE_HEADERS
    "${IMGUI_ROOT}/imconfig.h"
    "${IMGUI_ROOT}/imstb_rectpack.h"
    "${IMGUI_ROOT}/imstb_textedit.h"
    "${IMGUI_ROOT}/imstb_truetype.h")
set_target_properties(imgui PROPERTIES
                      PUBLIC_HEADER  "${IMGUI_ROOT}/imgui.h"
                      PRIVATE_HEADER "${IMGUI_PRIVATE_HEADERS}")
install(TARGETS imgui)
]])

# This target is OpenGL-Windows-specific.
if(SALT_TARGET_GRAPHICS STREQUAL "OpenGL" AND SALT_TARGET_OS STREQUAL "Windows")
    file(APPEND "${CMAKE_BINARY_DIR}/generated/libs/imgui/CMakeLists.txt"
    [[add_library(imgui_opengl_win STATIC)
    target_include_directories(imgui_opengl_win PRIVATE "${IMGUI_ROOT}" "${GLFW_ROOT}/include")
    target_sources(imgui_opengl_win PRIVATE
                   "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
                   "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp")
    set(IMGUI_OPENGL_WIN32_PRIVATE_HEADERS
        "${IMGUI_ROOT}/backends/imgui_impl_glfw.h"
        "${IMGUI_ROOT}/backends/imgui_impl_opengl3.h")
    set_target_properties(imgui_opengl_win PROPERTIES
                          PRIVATE_HEADER "${IMGUI_OPENGL_WIN32_PRIVATE_HEADERS}")
    install(TARGETS imgui_opengl_win)
    ]])
endif()

# This target is Metal-MacOSX-specific.
if(SALT_TARGET_GRAPHICS STREQUAL "Metal" AND SALT_TARGET_OS STREQUAL "MacOSX")
    file(APPEND "${CMAKE_BINARY_DIR}/generated/libs/imgui/CMakeLists.txt"
    [[enable_language(OBJCXX)

    add_library(imgui_metal_macosx STATIC)
    target_include_directories(imgui_metal_macosx PRIVATE "${IMGUI_ROOT}")
    target_compile_options(imgui_metal_macosx PRIVATE -fobjc-arc)
    target_sources(imgui_metal_macosx PRIVATE
                   "${IMGUI_ROOT}/backends/imgui_impl_metal.mm"
                   "${IMGUI_ROOT}/backends/imgui_impl_osx.mm")
    set(IMGUI_MACOSX_PRIVATE_HEADERS
        "${IMGUI_ROOT}/backends/imgui_impl_metal.h"
        "${IMGUI_ROOT}/backends/imgui_impl_osx.h")
    set_target_properties(imgui_metal_macosx PROPERTIES
                          PRIVATE_HEADER "${IMGUI_MACOSX_PRIVATE_HEADERS}")
    target_link_libraries(imgui_metal_macosx PRIVATE "-framework Metal" "-framework AppKit")
    install(TARGETS imgui_metal_macosx)
    ]])
endif()

message(" ==============================================================================\n"
        " Configuring Dear ImGui, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/libs/imgui
                        -S${CMAKE_BINARY_DIR}/generated/libs/imgui
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/libs/imgui
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF # Workaround for [LNK4217] warning
                        -DIMGUI_ROOT=${CMAKE_SOURCE_DIR}/libs/imgui
                        -DGLFW_ROOT=${CMAKE_SOURCE_DIR}/libs/glfw
                        -DGLAD_ROOT=${CMAKE_SOURCE_DIR}/cmake/modules
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
# Update CMakeCache.txt
########################################################################################################################

set(SALT_BOOTSTRAP_DONE TRUE CACHE BOOL "Whether the boostrapping has been done." FORCE)

# code: language='CMake' insertSpaces=true tabSize=4