add_library(salt::project_settings INTERFACE IMPORTED)

target_compile_features(salt::project_settings INTERFACE cxx_std_20)

# Enable output of compile commands during generation. This file will be used by clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Let CMake know where to find custom modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

#-----------------------------------------------------------------------------------------------------------------------
# Detect and initialize the target platform.
#-----------------------------------------------------------------------------------------------------------------------

# This function is required to get the correct target architecture because the CMAKE_SYSTEM_PROCESSOR
# variable does not always guarantee that it will correspond to the target architecture for the build.
# See: https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_PROCESSOR.html
function(salt_set_target_architecture output_var)
    # On MacOSX we use `CMAKE_OSX_ARCHITECTURES` *if* it was set.
    if(APPLE AND CMAKE_OSX_ARCHITECTURES)
        list(LENGTH CMAKE_OSX_ARCHITECTURES _ARCH_COUNT)
        if(_ARCH_COUNT STREQUAL "1")
            if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
                set(_SALT_TARGET_ARCH "x86_64" CACHE STRING "The target architecture." FORCE)
            else()
                message(FATAL_ERROR "Invalid target architecture. Salt engine only supports 64-bit architecture.")
            endif()
        else()
            message(FATAL_ERROR "Incorrectly initialized CMAKE_OSX_ARCHITECTURES variable.\n"
                                "Do not target multiple architectures at once.")
        endif()
    else()
        file(WRITE "${CMAKE_BINARY_DIR}/generated/arch/detect_arch.c"
        [[#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
        #   error TARGET_ARCH x86_64
        #if defined(__arm64) || defined(_M_ARM64) || defined(__aarch64__) || defined(__AARCH64EL__)
        #   error TARGET_ARCH arm64
        #endif
        #error TARGET_ARCH unsupported
        ]])

        enable_language(C)

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of `#ifdefs` that selects a particular `#error`
        # preprocessor directive whose message string contains the target architecture. The program will
        # always fail to compile (both because the file is not a valid C program, and obviously because of the
        # presence of the `#error` preprocessor directives... but by exploiting the preprocessor in this way,
        # we can detect the correct target architecture even when cross-compiling, since the program itself
        # never needs to be run (only the compiler/preprocessor).
        try_run(
            run_result_unused
            compile_result_unused
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/generated/arch/detect_arch.c"
            COMPILE_OUTPUT_VARIABLE _SALT_TARGET_ARCH
        )
        # Parse the architecture name from the compiler output.
        string(REGEX MATCH "TARGET_ARCH ([a-zA-Z0-9_]+)" _SALT_TARGET_ARCH "${_SALT_TARGET_ARCH}")

        # Get rid of the value marker leaving just the architecture name.
        string(REPLACE "TARGET_ARCH " "" _SALT_TARGET_ARCH "${_SALT_TARGET_ARCH}")

        if (_SALT_TARGET_ARCH STREQUAL "unsupported")
            message(FATAL_ERROR "Invalid target architecture. Salt engine only supports 64-bit architecture.")
        endif()
    endif()

    set(${output_var} "${_SALT_TARGET_ARCH}" PARENT_SCOPE)
endfunction(salt_set_target_architecture)

if(NOT (DEFINED CACHE{SALT_TARGET_CPU}    AND
        DEFINED CACHE{SALT_TARGET_OS}     AND
        DEFINED CACHE{SALT_TARGET_VENDOR} AND
        DEFINED CACHE{SALT_TARGET_GRAPHICS}))
    if(WIN32)
        if(NOT CMAKE_SYSTEM_VERSION)
            set(CMAKE_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION} CACHE STRING
                                                                  "The version of the target platform." FORCE)
        endif()

        if(NOT CMAKE_SYSTEM_PROCESSOR)
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "The target architecture." FORCE)
        endif()
        # Get the correct target architecture.
        salt_set_target_architecture(SALT_TARGET_CPU)

        set(SALT_TARGET_OS       Windows   CACHE STRING "[READONLY] The current platform."    FORCE)
        set(SALT_TARGET_VENDOR   Microsoft CACHE STRING "[READONLY] The target vendor."       FORCE)
        set(SALT_TARGET_GRAPHICS OpenGL    CACHE STRING "[READONLY] The target graphics api." FORCE)
    elseif(APPLE)
        if(NOT DEFINED CMAKE_OSX_SYSROOT)
            message(FATAL_ERROR "The required variable CMAKE_OSX_SYSROOT does not exist in CMake cache.\n"
                                "CMAKE_OSX_SYSROOT holds the path to the SDK.")
        endif()

        list(LENGTH CMAKE_OSX_ARCHITECTURES _ARCH_COUNT)
        if(_ARCH_COUNT STREQUAL "1")
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_OSX_ARCHITECTURES} CACHE STRING "The target architecture." FORCE)
        endif()

        if(CMAKE_OSX_SYSROOT MATCHES ".*/MacOSX.platform/*")
            if(NOT CMAKE_SYSTEM_VERSION)
                set(CMAKE_SYSTEM_VERSION 11.3 CACHE STRING "The version of the target platform." FORCE)
            endif()
            # Get the correct target architecture.
            salt_set_target_architecture(SALT_TARGET_CPU)

            set(SALT_TARGET_OS       MacOSX CACHE STRING "[READONLY] The current platform."    FORCE)
            set(SALT_TARGET_VENDOR   Apple  CACHE STRING "[READONLY] The target vendor."       FORCE)
            set(SALT_TARGET_GRAPHICS Metal  CACHE STRING "[READONLY] The target graphics api." FORCE)
        endif()
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Helper macros for imposing vendor/OS requirements on the built modules.
#-----------------------------------------------------------------------------------------------------------------------

macro(salt_requires_vendor _ARG_VENDOR)
    if(NOT SALT_TARGET_VENDOR STREQUAL ${_ARG_VENDOR})
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

function(salt_common_app _NAME)
    if(NOT SALT_TARGET_OS STREQUAL "Windows")
        message("Target '${_NAME}' is ignored because the target OS is set to '${SALT_TARGET_OS}'.")
        return()
    endif()
    cmake_parse_arguments(PARSE_ARGV 1          # start at the 1st argument
                          _SALT_COMMON_APP
                          ""                    # options
                          "BUNDLE_NAME"         # one   value keywords
                          "SOURCES;LINK")       # multi value keywords
    if(NOT _SALT_COMMON_APP_SOURCES)
        message(FATAL_ERROR "Target '${_NAME}' has no sources.\n"
                            "Perhaps you have forgotten to provide the SOURCES argument?")
    endif()
    add_executable("salt_${_NAME}")
    target_sources("salt_${_NAME}" PRIVATE "${_SALT_COMMON_APP_SOURCES}")
    target_link_libraries("salt_${_NAME}" PRIVATE salt::project_settings)
    if(_SALT_COMMON_APP_LINK)
        target_link_libraries("salt_${_NAME}" PRIVATE "${_SALT_COMMON_APP_LINK}")
    endif()
    install(TARGETS "salt_${_NAME}"
            RUNTIME DESTINATION "salt-${_NAME}")
endfunction(salt_common_app)

function(salt_macosx_app _NAME)
    if(NOT SALT_TARGET_OS STREQUAL "MacOSX")
        message("Target '${_NAME}' is ignored because the target OS is set to '${SALT_TARGET_OS}'.")
        return()
    endif()
    cmake_parse_arguments(PARSE_ARGV 1          # start at the 1st argument
                          _SALT_MACOSX_APP
                          ""                    # options
                          "BUNDLE_NAME"         # one   value keywords
                          "SOURCES;LINK")       # multi value keywords
    if(NOT _SALT_MACOSX_APP_SOURCES)
        message(FATAL_ERROR "Target '${_NAME}' has no sources.\n"
                            "Perhaps you have forgotten to provide the SOURCES argument?")
    endif()
    if(NOT _SALT_MACOSX_BUNDLE_NAME)
        set(_SALT_MACOSX_BUNDLE_NAME "${_NAME}")
    endif()
    add_executable("salt_${_NAME}" MACOSX_BUNDLE)
    set_target_properties("salt_${_NAME}" PROPERTIES
                          MACOSX_BUNDLE_BUNDLE_NAME "${_SALT_MACOSX_BUNDLE_NAME}")
    target_sources("salt_${_NAME}" PRIVATE "${_SALT_MACOSX_APP_SOURCES}")
    target_link_libraries("salt_${_NAME}" PRIVATE salt::project_settings)
    target_link_libraries("salt_${_NAME}" PRIVATE "-framework AppKit")
    if(_SALT_MACOSX_APP_LINK)
        target_link_libraries("salt_${_NAME}" PRIVATE "${_SALT_MACOSX_APP_LINK}")
    endif()
    install(TARGETS "salt_${_NAME}"
            BUNDLE DESTINATION "salt-${_NAME}")
endfunction(salt_macosx_app)

# This macro is used by `salt_static_library` and `salt_interface_library` functions. Don't call
# it unless you know what you are doing.
macro(_salt_unit_tests _ARG_NAME _TESTS_SOURCE _ARG_TEST_LINK)
    if(_TESTS_SOURCE)
        find_package(Catch2 REQUIRED)
        set(_TESTS "salt_${_ARG_NAME}_tests")
        add_executable("${_TESTS}")
        target_sources("${_TESTS}" PRIVATE "${_TESTS_SOURCE}")
        target_link_libraries("${_TESTS}"
                              PRIVATE Catch2::Catch2
                                      "salt::${_ARG_NAME}"
                                      "${_ARG_TEST_LINK}")
        install(TARGETS "${_TESTS}"
                RUNTIME DESTINATION "salt-${_ARG_NAME}/bin")
    endif()
endmacro(_salt_unit_tests)

# This macro is used by `salt_static_library` and `salt_interface_library` functions. Don't call
# it unless you know what you are doing.
macro(_salt_install_headers _ARG_NAME)
    install(DIRECTORY   "${CMAKE_CURRENT_LIST_DIR}/salt"
            DESTINATION "salt-${_ARG_NAME}/include"
            FILES_MATCHING PATTERN "*.hpp")
endmacro(_salt_install_headers)

# salt_static_library(<name>
#                     [<DEBUG>]
#                     [<MACOSX_SOURCE>    <item>...]
#                     [<MACOSX_TEST>      <item>...]
#                     [<COMMON_SOURCE>    <item>...]
#                     [<COMMON_TEST>      <item>...]
#                     [<PUBLIC_LINK>      <item>...]
#                     [<PRIVATE_LINK>     <item>...]
#                     [<TEST_LINK>        <item>...])
#
# Build and install a static library and its unit tests.
#
# The `DEBUG` option causes the function to print a debug message. You don't have to use this option
# unless you're trying to debug the funciton itself.
#
# The `MACOSX_SOURCE` option specifies a list of source files which are specific to MacOSX.
# The `MACOSX_TEST` option specifies a list of unit tests which are specific to MacOSX.
#
# The `COMMON_SOURCE` option specifies a list of platform-independent source files.
# The `COMMON_TEST` option specifies a list of platform-independent unit tests.
#
# The `PUBLIC_LINK` option specifies a list of libraries and targets which are link dependencies of the
# library. They are made part of its link interface.
#
# The `PRIVATE_LINK` option specifies a list of libraries and targets which are link dependencies of the
# library. They are not made part of its link interface.
#
# The `TEST_LINK` option specifies a list of libraries and targets which are link dependencies of the
# unit tests.
function(salt_static_library _ARG_NAME)
    set(_MULTI_VALUE_KEYWORDS
        MACOSX_SOURCE       # MacOSX-specific source code
        MACOSX_TEST         # MacOSX-specific unit tests
        COMMON_SOURCE       # common source code
        COMMON_TEST         # common unit tests
        PUBLIC_LINK         # public link libraries
        PRIVATE_LINK        # private link libraries
        TEST_LINK)          # tests-specific link libraries
    cmake_parse_arguments(PARSE_ARGV 1                  # start at the 1st argument
                          _ARG                          # variable prefix
                          "DEBUG"                       # options
                          ""                            # one value keywords
                          "${_MULTI_VALUE_KEYWORDS}")   # multi value kewywords
    if(_ARG_DEBUG)
        message(" (DEBUG) salt_static_library:\n"
                " NAME:             [${_ARG_NAME}]\n"
                " DEBUG:            [${_ARG_DEBUG}]\n"
                " MACOSX_SOURCE:    [${_ARG_MACOSX_SOURCE}]\n"
                " MACOSX_TEST:      [${_ARG_MACOSX_TEST}]\n"
                " COMMON_SOURCE:    [${_ARG_COMMON_SOURCE}]\n"
                " COMMON_TEST:      [${_ARG_COMMON_TEST}]\n"
                " PUBLIC_LINK:      [${_ARG_PUBLIC_LINK}]\n"
                " PRIVATE_LINK:     [${_ARG_PRIVATE_LINK}]\n"
                " TEST_LINK:        [${_ARG_TEST_LINK}]")
    endif()
    set(_TARGET_SOURCE "${_ARG_COMMON_SOURCE}")
    set(_TESTS_SOURCE  "${_ARG_COMMON_TEST}")
    if(SALT_TARGET_VENDOR STREQUAL "Apple")
        if(SALT_TARGET_OS STREQUAL "MacOSX")
            list(APPEND _TARGET_SOURCE "${_ARG_MACOSX_SOURCE}")
            list(APPEND _TESTS_SOURCE  "${_ARG_MACOSX_TEST}")
        endif()
    endif()
    set(_TARGET "salt_${_ARG_NAME}")
    add_library("${_TARGET}" STATIC)
    add_library("salt::${_ARG_NAME}" ALIAS "${_TARGET}")
    target_include_directories("${_TARGET}" PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
    target_link_libraries("${_TARGET}"
                          PUBLIC  salt::project_settings
                                  "${_ARG_PUBLIC_LINK}"
                          PRIVATE "${_ARG_PRIVATE_LINK}")
    target_sources("${_TARGET}" PRIVATE "${_TARGET_SOURCE}")
    install(TARGETS "${_TARGET}"
            ARCHIVE DESTINATION "salt-${_ARG_NAME}/lib")
    _salt_install_headers("${_ARG_NAME}")
    _salt_unit_tests("${_ARG_NAME}" "${_TESTS_SOURCE}" "${_ARG_TEST_LINK}")
endfunction(salt_static_library)

# salt_interface_library(<name>
#                        [<DEBUG>]
#                        [<MACOSX_TEST>      <item>...]
#                        [<COMMON_TEST>      <item>...]
#                        [<PUBLIC_LINK>      <item>...]
#                        [<TEST_LINK>        <item>...])
#
# Build and install an interface (aka header-only) library and its unit tests.
#
# The `DEBUG` option causes the function to print a debug message. You don't have to use this option
# unless you're trying to debug the funciton itself.
#
# The `MACOSX_TEST` option specifies a list of unit tests which are specific to MacOSX.
#
# The `COMMON_TEST` option specifies a list of platform-independent unit tests.
#
# The `PUBLIC_LINK` option specifies a list of libraries and targets which are link dependencies of the
# library. They are made part of its link interface.
#
# The `TEST_LINK` option specifies a list of libraries and targets which are link dependencies of the
# unit tests.
function(salt_interface_library _ARG_NAME)
    set(_MULTI_VALUE_KEYWORDS
        MACOSX_TEST         # MacOSX-specific unit tests
        COMMON_TEST         # common unit tests
        PUBLIC_LINK         # public link libraries
        TEST_LINK)          # tests-specific link libraries
    cmake_parse_arguments(PARSE_ARGV 1                  # start at the 1st argument
                          _ARG                          # variable prefix
                          "DEBUG"                       # options
                          ""                            # one value keywords
                          "${_MULTI_VALUE_KEYWORDS}")   # multi value kewywords
    if(_ARG_DEBUG)
        message(" (DEBUG) salt_static_library:\n"
                " NAME:             [${_ARG_NAME}]\n"
                " DEBUG:            [${_ARG_DEBUG}]\n"
                " MACOSX_TEST:      [${_ARG_MACOSX_TEST}]\n"
                " COMMON_TEST:      [${_ARG_COMMON_TEST}]\n"
                " PUBLIC_LINK:      [${_ARG_PUBLIC_LINK}]\n"
                " TEST_LINK:        [${_ARG_TEST_LINK}]")
    endif()
    set(_TESTS_SOURCE  "${_ARG_COMMON_TEST}")
    if(SALT_TARGET_VENDOR STREQUAL "Apple")
        if(SALT_TARGET_OS STREQUAL "MacOSX")
            list(APPEND _TESTS_SOURCE  "${_ARG_MACOSX_TEST}")
        endif()
    endif()
    set(_TARGET "salt_${_ARG_NAME}")
    add_library("${_TARGET}" INTERFACE)
    add_library("salt::${_ARG_NAME}" ALIAS "${_TARGET}")
    target_include_directories("${_TARGET}" INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
    target_link_libraries("${_TARGET}"
                          INTERFACE salt::project_settings
                                    "${_ARG_PUBLIC_LINK}")
    _salt_install_headers("${_ARG_NAME}")
    _salt_unit_tests("${_ARG_NAME}" "${_TESTS_SOURCE}" "${_ARG_TEST_LINK}")
endfunction(salt_interface_library)

#-----------------------------------------------------------------------------------------------------------------------
# Build type.
#-----------------------------------------------------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

# Possible values of build type for cmake-gui and ccmake.
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
string(TOUPPER ${CMAKE_BUILD_TYPE} SALT_BUILD_TYPE)

#-----------------------------------------------------------------------------------------------------------------------
# Setting a properly used compiler, including using a different frontend.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?(C|c)?lang$")
    target_compile_definitions(salt::project_settings INTERFACE SALT_CLANG)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(salt::project_settings INTERFACE SALT_GNUC)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_definitions(salt::project_settings INTERFACE SALT_MSVC)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Setting the correct CMAKE_<LANG>_FLAGS_<BUILD_TYPE> variables for the Clang-Windows bundle.
#-----------------------------------------------------------------------------------------------------------------------

# The code below changes the CMAKE_<LANG>_FLAGS_<BUILD_TYPE> variables. It does this for a good reason.
# Don't do this in normal code. Instead add the necessary compile/linker flags to salt::project_settings.
if(SALT_TARGET_OS STREQUAL "Windows" AND CMAKE_CXX_COMPILER_ID MATCHES "(C|c)lang")
    if(SALT_BUILD_TYPE STREQUAL "DEBUG")
        string(APPEND CMAKE_C_FLAGS_${SALT_BUILD_TYPE}   " -D_DEBUG -D_MT -Xclang --dependent-lib=msvcrtd")
        string(APPEND CMAKE_CXX_FLAGS_${SALT_BUILD_TYPE} " -D_DEBUG -D_MT -Xclang --dependent-lib=msvcrtd")
    else()
        string(APPEND CMAKE_C_FLAGS_${SALT_BUILD_TYPE}   " -D_MT -Xclang --dependent-lib=msvcrt")
        string(APPEND CMAKE_CXX_FLAGS_${SALT_BUILD_TYPE} " -D_MT -Xclang --dependent-lib=msvcrt")
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Getting LLVM Bitcode.
#-----------------------------------------------------------------------------------------------------------------------

# The code below changes the CMAKE_<LANG>_FLAGS and CMAKE_<LANG>_LINK_FLAGS variables. It does this for a good reason.
# Don't do this in normal code. Instead add the necessary compile/linker flags to salt::project_settings.
if (SALT_TARGET_OS STREQUAL "MacOSX")
    option(SALT_ENABLE_BITCODE "Enable Bitcode generation." YES)

    if(SALT_ENABLE_BITCODE)
        string(APPEND CMAKE_C_FLAGS       " -fembed-bitcode")
        string(APPEND CMAKE_CXX_FLAGS     " -fembed-bitcode")
        string(APPEND CMAKE_OBJC_FLAGS    " -fembed-bitcode")
        string(APPEND CMAKE_OBJCXX_FLAGS  " -fembed-bitcode")

        # The flag '-headerpad_max_install_names' should not be used with '-fembed-bitcode'. CMake always adds
        # '-headerpad_max_install_names' flag. There's no appernt way to disable this flag otherwise.
        #   See: https://github.com/Kitware/CMake/blob/master/Modules/Platform/Darwin.cmake
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_C_LINK_FLAGS      ${CMAKE_C_LINK_FLAGS})
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_CXX_LINK_FLAGS    ${CMAKE_CXX_LINK_FLAGS})
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_OBJC_LINK_FLAGS   ${CMAKE_C_LINK_FLAGS})
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_OBJCXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Link time optimization.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE MATCHES "Release")
    option(SALT_ENABLE_LTO "Enable link time optimization. This is only valid for Release builds." YES)
endif()

if(SALT_ENABLE_LTO)
    # The code below changes the CMAKE_<LANG>_FLAGS and CMAKE_<LANG>_LINK_FLAGS variables. It does this for a good
    # reason. Don't do this in normal code. Instead add the necessary compile/linker flags to salt::project_settings.

    # For LTO to work, we have to pass `-flto` flag to the compiler both at compile...
    string(APPEND CMAKE_C_FLAGS   " -flto")
    string(APPEND CMAKE_CXX_FLAGS " -flto")

    # and link time.
    string(APPEND CMAKE_C_LINKER_FLAGS   " -flto")
    string(APPEND CMAKE_CXX_LINKER_FLAGS " -flto")

    if(SALT_TARGET_OS STREQUAL "MacOSX")
        # And the same thing for Obj-C/CXX compiler...
        string(APPEND CMAKE_OBJC_FLAGS   " -flto")
        string(APPEND CMAKE_OBJCXX_FLAGS " -flto")
        
        # and linker.
        string(APPEND CMAKE_OBJC_LINKER_FLAGS   " -flto")
        string(APPEND CMAKE_OBJCXX_LINKER_FLAGS " -flto")
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Enable colored diagnostics.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?(C|c)?lang$")
    target_compile_options(salt::project_settings INTERFACE -fcolor-diagnostics)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(salt::project_settings INTERFACE -fdiagnostics-color=always)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Compiler cache.
#-----------------------------------------------------------------------------------------------------------------------

find_program(SALT_CCACHE_COMMAND ccache)

if(SALT_CCACHE_COMMAND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SALT_CCACHE_COMMAND})
else()
    message(WARNING "Cannot find ccache. Incremental builds may get slower.")
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Warnings.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "(^(Apple)?(C|c)?lang$)|GNU")
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
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # TODO:
    #   Add compile flags for MSVC.
endif()

option(SALT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." YES)

if(SALT_WARNINGS_AS_ERRORS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "(^(Apple)?(C|c)?lang$)|GNU")
        target_compile_options(salt::project_settings INTERFACE
            # Make all warnings into errors.
            -Werror)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(salt::project_settings INTERFACE
            # Make all warnings into errors.
            #/WX
            )
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Sanitizers.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "(^(Apple)?(C|c)?lang$)|GNU" AND NOT SALT_TARGET_OS STREQUAL "Windows")
    option(SALT_ENABLE_SANITIZER_ADDRESS   "Enable address sanitizer."            YES)
    option(SALT_ENABLE_SANITIZER_THREAD    "Enable thread  sanitizer."            NO )
    option(SALT_ENABLE_SANITIZER_UNDEFINED "Enable undefined behavior sanitizer." YES)
    option(SALT_ENABLE_SANITIZER_LEAK      "Enable leak sanitizer."               NO )

    if((SALT_ENABLE_SANITIZER_LEAK OR SALT_ENABLE_SANITIZER_ADDRESS) AND SALT_ENABLE_SANITIZER_THREAD)
        message(WARNING "Thread sanitizer does not work with Address or Leak sanitizer enabled.")
    endif()

    set(SALT_SANITIZERS "")

    if(SALT_ENABLE_SANITIZER_ADDRESS)
        list(APPEND SALT_SANITIZERS "address")
    endif()

    if(SALT_ENABLE_SANITIZER_THREAD)
        list(APPEND SALT_SANITIZERS "thread")
    endif()

    if(SALT_ENABLE_SANITIZER_UNDEFINED)
        list(APPEND SALT_SANITIZERS "undefined")
    endif()

    if(SALT_ENABLE_SANITIZER_LEAK)
        list(APPEND SALT_SANITIZERS "leak")
    endif()

    list(JOIN SALT_SANITIZERS "," SALT_ENABLED_SANITIZERS)

    target_compile_options(salt::project_settings INTERFACE -fsanitize=${SALT_ENABLED_SANITIZERS})

    target_link_options(salt::project_settings INTERFACE -fsanitize=${SALT_ENABLED_SANITIZERS})
endif()

# code: language="CMake" insertSpaces=true tabSize=4