#!/usr/bin/env bash
source ${BASH_SOURCE[0]%/*}/common.sh

#######################################################################################################################
readonly NINJA_EXECUTABLE=`which ninja`

if [ "x${NINJA_EXECUTABLE}" == "x" ]; then
    fail "Cannot find Ninja."
fi

#######################################################################################################################
readonly CLANG_EXECUTABLE=`which clang`

if [ "x${CLANG_EXECUTABLE}" == "x" ]; then
    fail "Cannot find Clang."
fi

#######################################################################################################################
readonly CLANGXX_EXECUTABLE=`which clang++`

if [ "x${CLANGXX_EXECUTABLE}" == "x" ]; then
    fail "Cannot find Clang++."
fi

#######################################################################################################################
# Generate the CMakePresets.json

# detect_os detects target OS.
detect_os() {
    case "$OSTYPE" in
        darwin*)  echo "MacOSX";; 
        linux*)   echo "Linux";;
        *)        echo "Unsupported";;
    esac
}

# apple_min_os_version returns the minimum supported version of the given OS/SDK.
apple_min_os_version() {
    case $1 in
        macosx)     echo 11;;
        iphoneos)   echo 13;;
        iphonesimulator)  echo 13;;
    esac
}

# generate_apple_cmake_presets generates a portion of cmake-variants.yaml for the given OS and architectures.
generate_apple_cmake_presets() {
    OS=$1
    GRAPHICS=$2
    DESCRIPTION=$3

    for ARCH in "${@:4}"; do
cat <<EOF
        {
            "name":         "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-debug",
            "displayName":  "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-debug",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA_EXECUTABLE}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "$(xcrun --sdk ${OS} --find clang++ 2>/dev/null)",
                "CMAKE_C_COMPILER":             "$(xcrun --sdk ${OS} --find clang   2>/dev/null)",
                "CMAKE_OBJCXX_COMPILER":        "$(xcrun --sdk ${OS} --find clang++ 2>/dev/null)",
                "CMAKE_OBJC_COMPILER":          "$(xcrun --sdk ${OS} --find clang   2>/dev/null)",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "CMAKE_OSX_DEPLOYMENT_TARGET":  "$(apple_min_os_version ${OS})",
                "CMAKE_OSX_SYSROOT":            "$(xcrun --sdk ${OS} --show-sdk-path 2>/dev/null)",
                "SALT_GRAPHICS":                "${GRAPHICS}"
            }
        },
        {
            "name":         "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-release",
            "displayName":  "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-release",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA_EXECUTABLE}",
                "CMAKE_BUILD_TYPE":             "RelWithDebInfo",
                "CMAKE_CXX_COMPILER":           "$(xcrun --sdk ${OS} --find clang++ 2>/dev/null)",
                "CMAKE_C_COMPILER":             "$(xcrun --sdk ${OS} --find clang   2>/dev/null)",
                "CMAKE_OBJCXX_COMPILER":        "$(xcrun --sdk ${OS} --find clang++ 2>/dev/null)",
                "CMAKE_OBJC_COMPILER":          "$(xcrun --sdk ${OS} --find clang   2>/dev/null)",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "CMAKE_OSX_DEPLOYMENT_TARGET":  "$(apple_min_os_version ${OS})",
                "CMAKE_OSX_SYSROOT":            "$(xcrun --sdk ${OS} --show-sdk-path 2>/dev/null)",
                "SALT_GRAPHICS":                "${GRAPHICS}"
            }
        },
EOF
    done
}

# generate_linux_cmake_presets generates a portion of cmake-variants.yaml for the given OS and architectures.
generate_linux_cmake_presets() {
    OS=$1
    GRAPHICS=$2
    DESCRIPTION=$3

    for ARCH in "${@:4}"; do
        if [[ "${ARCH}" == "arm64" ]]; then
            continue
        fi
cat <<EOF
        {
            "name":         "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-debug",
            "displayName":  "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-debug",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA_EXECUTABLE}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "${CLANGXX_EXECUTABLE}",
                "CMAKE_C_COMPILER":             "${CLANG_EXECUTABLE}",
                "CMAKE_OBJCXX_COMPILER":        "${CLANGXX_EXECUTABLE}",
                "CMAKE_OBJC_COMPILER":          "${CLANG_EXECUTABLE}",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "SALT_GRAPHICS":                "${GRAPHICS}"
            }
        },
        {
            "name":         "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-release",
            "displayName":  "${OS}-${ARCH}-$(tr '[:upper:]' '[:lower:]'<<<${GRAPHICS})-release",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA_EXECUTABLE}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "${CLANGXX_EXECUTABLE}",
                "CMAKE_C_COMPILER":             "${CLANG_EXECUTABLE}",
                "CMAKE_OBJCXX_COMPILER":        "${CLANGXX_EXECUTABLE}",
                "CMAKE_OBJC_COMPILER":          "${CLANG_EXECUTABLE}",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "SALT_GRAPHICS":                "${GRAPHICS}"
            }
        },
EOF
    done
}

# generate_cmake_presets generates a portion of cmake-variants.yaml for the given OS and architectures.
generate_cmake_presets() {
    OS=$1
    GRAPHICS=$2

    if [ "${GRAPHICS}" != "OpenGL" ]; then
        DESCRIPTION="This configuration is temporarily unsupported."
    fi

    if [ "${OS}" == "MacOSX" ]; then        
        generate_apple_cmake_presets macosx "${GRAPHICS}" "${DESCRIPTION}" "${@:3}"
    elif [ "${OS}" == "Linux" ]; then
        generate_linux_cmake_presets linux  "${GRAPHICS}" "${DESCRIPTION}" "${@:3}"
    fi
}

if [ "$(detect_os)" == "Unsupported" ]; then
    fail "Unsupported OS: $OSTYPE."
fi

# Will generate CMakePresets.json that is specific to your development environment, so that VSCode will be able
# to configure VSCode CMake Tools correctly.
cat <<EOF >${PROJECT_ROOT}/CMakePresets.json
{
    "version": 3,
    "configurePresets": [
$(generate_cmake_presets $(detect_os) OpenGL arm64 x86_64)
$(generate_cmake_presets $(detect_os) Metal  arm64 x86_64)
$(generate_cmake_presets $(detect_os) Vulkan arm64 x86_64)
        {
            "name":         "base",
            "description":  "For more information: http://aka.ms/cmakepresetsvs",
            "hidden":       true,
            "generator":    "Ninja",
            "binaryDir":    "\${sourceDir}/build/\${presetName}",
            "installDir":   "\${sourceDir}/build/\${presetName}/output/"
        }
    ]
}
EOF