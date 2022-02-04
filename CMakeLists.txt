cmake_minimum_required(VERSION 3.20)

project(salt CXX)

include("cmake/project_settings.cmake")
include("cmake/bootstrap.cmake")

#-----------------------------------------------------------------------------------------------------------------------
# Libraries.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-config")
add_subdirectory("salt-core")
add_subdirectory("salt-events")
add_subdirectory("salt-math")
add_subdirectory("salt-meta")
add_subdirectory("salt-platform")
add_subdirectory("salt-utils")

#-----------------------------------------------------------------------------------------------------------------------
# Applications.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-playground-macos")
add_subdirectory("salt-playground-windows")

# code: language="CMake" insertSpaces=true tabSize=4