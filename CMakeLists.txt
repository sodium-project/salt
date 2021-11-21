cmake_minimum_required(VERSION 3.20)

project(salt CXX)

include("cmake/project_settings.cmake")
include("cmake/boostrap.cmake")

#-----------------------------------------------------------------------------------------------------------------------
# Libraries.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-config")
add_subdirectory("salt-core")
add_subdirectory("salt-foundation")
add_subdirectory("salt-meta")

#-----------------------------------------------------------------------------------------------------------------------
# Applications.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-playground-macos")
add_subdirectory("salt-playground-windows")

# code: language="CMake" insertSpaces=true tabSize=4