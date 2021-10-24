add_library(salt::project_settings INTERFACE IMPORTED)

target_compile_features(salt::project_settings INTERFACE cxx_std_20)

# Enable output of compile commands during generation. This file will be used by clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Let CMake know where to find custom modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

#-----------------------------------------------------------------------------------------------------------------------
# Detect and initialize the target platform.
#-----------------------------------------------------------------------------------------------------------------------

if(NOT (DEFINED CACHE{SALT_TARGET_CPU}    AND
        DEFINED CACHE{SALT_TARGET_OS}     AND
        DEFINED CACHE{SALT_TARGET_VENDOR} AND
        DEFINED CACHE{SALT_TARGET_GRAPHICS}))
    if(WIN32)
        if(NOT CMAKE_SYSTEM_VERSION)
            set(CMAKE_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION} CACHE STRING "The version of the target platform." FORCE)
        endif()

        if(NOT CMAKE_SYSTEM_PROCESSOR)
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "The target architecture." FORCE)
        endif()

        set(SALT_TARGET_CPU      ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "[READONLY] The target CPU."          FORCE)
        set(SALT_TARGET_OS       Windows                   CACHE STRING "[READONLY] The current platform."    FORCE)
        set(SALT_TARGET_VENDOR   Microsoft                 CACHE STRING "[READONLY] The target vendor."       FORCE)
        set(SALT_TARGET_GRAPHICS OpenGL                    CACHE STRING "[READONLY] The target graphics api." FORCE)
    elseif(APPLE)
        if(NOT DEFINED CMAKE_OSX_SYSROOT)
            message(FATAL_ERROR " The required variable CMAKE_OSX_SYSROOT does not exist in CMake cache.\n"
                                " CMAKE_OSX_SYSROOT holds the path to the SDK.\n")
        endif()

        list(LENGTH CMAKE_OSX_ARCHITECTURES _ARCH_COUNT)
        if(_ARCH_COUNT STREQUAL "1")
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_OSX_ARCHITECTURES} CACHE STRING "The target architecture." FORCE)
        endif()

        if(CMAKE_OSX_SYSROOT MATCHES ".*/MacOSX.platform/*")
            if(NOT CMAKE_SYSTEM_VERSION)
                set(CMAKE_SYSTEM_VERSION 11.3 CACHE STRING "The version of the target platform." FORCE)
            endif()
            if(NOT CMAKE_SYSTEM_PROCESSOR)
                set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "The target architecture." FORCE)
            endif()
            set(SALT_TARGET_CPU      ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "[READONLY] The target CPU."          FORCE)
            set(SALT_TARGET_OS       MacOSX                    CACHE STRING "[READONLY] The current platform."    FORCE)
            set(SALT_TARGET_VENDOR   Apple                     CACHE STRING "[READONLY] The target vendor."       FORCE)
            set(SALT_TARGET_GRAPHICS Metal                     CACHE STRING "[READONLY] The target graphics api." FORCE)
        endif()
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Helper macros for imposing vendor/OS requirements on the built modules.
#-----------------------------------------------------------------------------------------------------------------------

macro(salt_requires_vendor _ARG_VENDOR)
    if (NOT SALT_TARGET_VENDOR STREQUAL ${_ARG_VENDOR})
        get_filename_component(_TMP_BASENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
        message("The subdirectory '${_TMP_BASENAME}' is ignored because the target OS vendor is set to "
                "'${SALT_TARGET_VENDOR}'")
        unset(_TMP_BASENAME)
        return()
    endif()
endmacro(salt_requires_vendor)

#-----------------------------------------------------------------------------------------------------------------------
# Installation.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Defines the directory where the build artifacts will be placed.
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output" CACHE PATH "" FORCE)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Helper functions.
#-----------------------------------------------------------------------------------------------------------------------

function(salt_win32_app _NAME)
    if(NOT SALT_TARGET_OS STREQUAL "Windows")
        message("Target '${_NAME}' is ignored because the target OS is set to '${SALT_TARGET_OS}'.")
        return()
    endif()
    cmake_parse_arguments(PARSE_ARGV 1          # start at the 1st argument
                          _SALT_WIN32_APP
                          ""                    # options
                          "BUNDLE_NAME"         # one   value keywords
                          "SOURCES;LINK")       # multi value keywords
    if(NOT _SALT_WIN32_APP_SOURCES)
        message(FATAL_ERROR " Target '${_NAME}' has no sources.\n"
                            " Perhaps you have forgotten to provide the SOURCES argument?")
    endif()
    set(SALT_APP_NAME "salt_${_NAME}")
    add_executable("${SALT_APP_NAME}")
    target_sources("${SALT_APP_NAME}" PRIVATE "${_SALT_WIN32_APP_SOURCES}")
    target_link_libraries("${SALT_APP_NAME}" PRIVATE salt::project_settings)
    if(_SALT_WIN32_APP_LINK)
        target_link_libraries("${SALT_APP_NAME}" PRIVATE "${_SALT_WIN32_APP_LINK}")
    endif()
    install(TARGETS "${SALT_APP_NAME}" RUNTIME DESTINATION "salt-${_NAME}")
endfunction(salt_win32_app)

#-----------------------------------------------------------------------------------------------------------------------
# Build type.
#-----------------------------------------------------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

# Possible values of build type for cmake-gui and ccmake.
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

#-----------------------------------------------------------------------------------------------------------------------
# Enable colored diagnostics.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER MATCHES ".*Clang")
    target_compile_options(salt::project_settings INTERFACE -fcolor-diagnostics)
endif()

if(CMAKE_CXX_COMPILER MATCHES "GNU")
    target_compile_options(salt::project_settings INTERFACE -fdiagnostics-color=always)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Warnings.
#-----------------------------------------------------------------------------------------------------------------------

target_compile_options(salt::project_settings INTERFACE
    # Enable all the warnings about constructions that some users consider questionable, and that are easy to avoid.
    -Wall
    # Enable some extra warnings that are not enabled by -Wall.
    -Wextra
    # Warn whenever a local variable or type shadows another one.
    -Wshadow
    # Warn whenever a class has virtual functions and an accessible non-virtual destructor.
    -Wnon-virtual-dtor
    # Warn if old-style (C-style) cast to a non-void type is used within a C++ program.
    -Wold-style-cast
    # Warn whenever a pointer is cast such that the required alignment of the target is increased. For example, warn if
    # a char* is cast to an int* on machines where integers can only be accessed at two- or four-byte boundaries.
    -Wcast-align
    # Warn on anything being unused.
    -Wunused
    # Warn when a function declaration hides virtual functions from a base class.
    -Woverloaded-virtual
    # Warn whenever non-standard C++ is used.
    -Wpedantic
    # Warn on implicit conversions that may alter a value. This includes conversions between real and integer,
    # like abs(x) when x is double.
    -Wconversion
    # Warn for implicit conversions that may change the sign of an integer value, like assigning a signed integer
    # expression to an unsigned integer variable.
    -Wsign-conversion
    # Warn if the compiler detects paths that trigger erroneous or undefined behaviour due to dereferencing a null
    # pointer.
    -Wnull-dereference
    # Warn whenever a value of type float is implicitly promoted to double.
    -Wdouble-promotion
    # Check calls to printf and scanf, etc., to make sure that the arguments supplied have types appropriate to the
    # format string.
    -Wformat=2)

option(SALT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." YES)

if(SALT_WARNINGS_AS_ERRORS)
    target_compile_options(salt::project_settings INTERFACE
        # Make all warnings into errors.
        -Werror)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Sanitizers.
#-----------------------------------------------------------------------------------------------------------------------

option(SALT_ENABLE_SANITIZER_ADDRESS   "Enable address sanitizer."            YES)
option(SALT_ENABLE_SANITIZER_THREAD    "Enable thread  sanitizer."            NO )
option(SALT_ENABLE_SANITIZER_UNDEFINED "Enable undefined behavior sanitizer." YES)
option(SALT_ENABLE_SANITIZER_LEAK      "Enable leak sanitizer."               NO )

if((ENABLE_SANITIZER_LEAK OR ENABLE_SANITIZER_ADDRESS) AND ENABLE_SANITIZER_THREAD)
    message(WARNING "Thread sanitizer does not work with Address or Leak sanitizer enabled.")
endif()

set(SANITIZERS "")

if(ENABLE_SANITIZER_ADDRESS)
    list(APPEND SANITIZERS "address")
endif()

if(ENABLE_SANITIZER_THREAD)
    list(APPEND SANITIZERS "thread")
endif()

if(ENABLE_SANITIZER_UNDEFINED)
    list(APPEND SANITIZERS "undefined")
endif()

if(ENABLE_SANITIZER_LEAK)
    list(APPEND SANITIZERS "leak")
endif()

list(JOIN SANITIZERS "," ENABLED_SANITIZERS)

target_compile_options(salt::project_settings INTERFACE -fsanitize=${ENABLED_SANITIZERS})

target_link_options(salt::project_settings INTERFACE -fsanitize=${ENABLED_SANITIZERS})