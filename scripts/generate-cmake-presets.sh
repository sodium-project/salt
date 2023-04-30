#!/usr/bin/env bash
source ${BASH_SOURCE[0]%/*}/common.sh

#######################################################################################################################
# Detect target OS and Clang version.

detect_os() {
    case "$OSTYPE" in
        darwin*)  echo "MacOSX";;
        linux*)   echo "Linux";;
        *)        echo "Unsupported";;
    esac
}

detect_clang() {
    readonly CLANG=$(which clang)
    if [ "x${CLANG}" == "x" ]; then
        fail "Cannot find Clang. Please make sure it is available in your PATH."
    fi

    readonly CLANGXX=$(which clang++)
    if [ "x${CLANGXX}" == "x" ]; then
        fail "Cannot find Clang++. Please make sure it is available in your PATH."
    fi

    readonly CLANG_MAJOR=$(echo __clang_major__      | clang -E -x c - | tail -n 1)
    readonly CLANG_MINOR=$(echo __clang_minor__      | clang -E -x c - | tail -n 1)
    readonly CLANG_PATCH=$(echo __clang_patchlevel__ | clang -E -x c - | tail -n 1)

    if [[ "${CLANG_MAJOR}" -lt 16 ]]; then
        fail "The minimum supported version of Clang is 16 or higher. Please update Clang to the minimum supported version."
    fi

    echo "Detected clang version: ${CLANG_MAJOR}.${CLANG_MINOR}.${CLANG_PATCH}"
}

if [ "$(detect_os)" == "Unsupported" ]; then
    fail "Unsupported OS: $OSTYPE."
elif [ "$(detect_os)" == "Linux" ]; then    
    detect_clang # For MacOS, the detection method will probably need to be revised.
fi

#######################################################################################################################
readonly NINJA=$(which ninja)

if [ "x${NINJA}" == "x" ]; then
    fail "Cannot find Ninja. Please make sure it is available in your PATH."
fi

#######################################################################################################################
# Generate the CMakePresets.json

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
    local readonly OS=$1
    local readonly GRAPHICS=$2
    local readonly DESCRIPTION=$3

    for ARCH in "${@:4}"; do
cat <<EOF
        {
            "name":         "${OS}-${ARCH}-$(lo ${GRAPHICS})-debug",
            "displayName":  "${OS}-${ARCH}-$(lo ${GRAPHICS})-debug",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA}",
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
            "name":         "${OS}-${ARCH}-$(lo ${GRAPHICS})-release",
            "displayName":  "${OS}-${ARCH}-$(lo ${GRAPHICS})-release",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA}",
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
    local readonly OS=$1
    local readonly GRAPHICS=$2
    local readonly DESCRIPTION=$3

    for ARCH in "${@:4}"; do
        # TODO:
        #   It will be necessary to check the arm64 build for Linux. Let's skip that for now.
        if [[ "${ARCH}" == "arm64" ]]; then
            continue
        fi
cat <<EOF
        {
            "name":         "${OS}-${ARCH}-$(lo ${GRAPHICS})-debug",
            "displayName":  "${OS}-${ARCH}-$(lo ${GRAPHICS})-debug",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "${CLANGXX}",
                "CMAKE_C_COMPILER":             "${CLANG}",
                "CMAKE_OBJCXX_COMPILER":        "${CLANGXX}",
                "CMAKE_OBJC_COMPILER":          "${CLANG}",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "SALT_GRAPHICS":                "${GRAPHICS}"
            }
        },
        {
            "name":         "${OS}-${ARCH}-$(lo ${GRAPHICS})-release",
            "displayName":  "${OS}-${ARCH}-$(lo ${GRAPHICS})-release",
            "description":  "${DESCRIPTION}",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "${CLANGXX}",
                "CMAKE_C_COMPILER":             "${CLANG}",
                "CMAKE_OBJCXX_COMPILER":        "${CLANGXX}",
                "CMAKE_OBJC_COMPILER":          "${CLANG}",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "SALT_GRAPHICS":                "${GRAPHICS}"
            }
        },
EOF
    done
}

# generate_cmake_presets generates a portion of cmake-variants.yaml for the given OS and architectures.
generate_cmake_presets() {
    local readonly OS=$1
    local readonly GRAPHICS=$2

    local DESCRIPTION=""
    if [ "${GRAPHICS}" != "OpenGL" ]; then
        DESCRIPTION+="This configuration is temporarily unsupported."
    fi

    if [ "${OS}" == "MacOSX" ] && [ "${GRAPHICS}" != "Vulkan" ]; then
        generate_apple_cmake_presets "$(lo ${OS})" "${GRAPHICS}" "${DESCRIPTION}" "${@:3}"
    elif [ "${OS}" == "Linux" ] && [ "${GRAPHICS}" != "Metal" ]; then
        generate_linux_cmake_presets "$(lo ${OS})" "${GRAPHICS}" "${DESCRIPTION}" "${@:3}"
    fi
}

echo "Generating presets.."

# Will generate CMakePresets.json that is specific to your development environment, so that VSCode will be able
# to configure VSCode CMake Tools correctly.
cat <<EOF >${PROJECT_ROOT}/CMakePresets.json
{
    "version": 3,
    "configurePresets": [
$(generate_cmake_presets $(detect_os) OpenGL arm64 x86_64)
$(generate_cmake_presets $(detect_os) Vulkan arm64 x86_64)
$(generate_cmake_presets $(detect_os) Metal  arm64 x86_64)
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

echo "Successfully generated presets in ${PROJECT_ROOT}/CMakePresets.json"

# code: language='Shell Script' insertSpaces=true tabSize=4