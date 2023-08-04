cmake_minimum_required(VERSION 3.23)

project(salt LANGUAGES CXX)

include("cmake/project_settings.cmake")
include("cmake/bootstrap.cmake")

#-----------------------------------------------------------------------------------------------------------------------
# Enable code coverage.
#-----------------------------------------------------------------------------------------------------------------------

option(SALT_ENABLE_COVERAGE "Enable code coverage" OFF)

if(SALT_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?(C|c)?lang$")
        string(APPEND CMAKE_C_FLAGS   " -O0 -g -fprofile-instr-generate -fcoverage-mapping")
        string(APPEND CMAKE_CXX_FLAGS " -O0 -g -fprofile-instr-generate -fcoverage-mapping")

        message(STATUS "Code coverage: ON")
    else()
        message(ERROR "Code coverage for compiler ${CMAKE_CXX_COMPILER_ID} is unsupported")
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Libraries.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-config")
add_subdirectory("salt-core")
add_subdirectory("salt-foundation")
add_subdirectory("salt-meta")
add_subdirectory("salt-platform")

#-----------------------------------------------------------------------------------------------------------------------
# Applications.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-playground")

# code: language="CMake" insertSpaces=true tabSize=4