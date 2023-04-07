cmake_minimum_required(VERSION 3.20)

#-----------------------------------------------------------------------------------------------------------------------
# Set cmake export compile commands.
#-----------------------------------------------------------------------------------------------------------------------

if(NOT DEFINED CMAKE_EXPORT_COMPILE_COMMANDS AND
   NOT DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
  set(CMAKE_EXPORT_COMPILE_COMMANDS "ON"
      CACHE BOOL "Enable/Disable output of compile commands during generation.")
  mark_as_advanced(CMAKE_EXPORT_COMPILE_COMMANDS)

  message(STATUS "CMAKE_EXPORT_COMPILE_COMMANDS: ${CMAKE_EXPORT_COMPILE_COMMANDS}")
endif()

project(salt CXX)

include("cmake/project_settings.cmake")
include("cmake/bootstrap.cmake")
include("cmake/std_compatibility.cmake")

#-----------------------------------------------------------------------------------------------------------------------
# Libraries.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-config")
add_subdirectory("salt-core")
add_subdirectory("salt-events")
add_subdirectory("salt-foundation")
add_subdirectory("salt-math")
add_subdirectory("salt-memory")
add_subdirectory("salt-meta")
add_subdirectory("salt-platform")

#-----------------------------------------------------------------------------------------------------------------------
# Applications.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("salt-playground-macos")
add_subdirectory("salt-playground-windows")

# code: language="CMake" insertSpaces=true tabSize=4