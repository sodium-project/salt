#!/usr/bin/env bash
source ${BASH_SOURCE[0]%/*}/common.sh

#######################################################################################################################
readonly NINJA_EXECUTABLE=`which ninja`

if [ "x${NINJA_EXECUTABLE}" == "x" ]; then
    fail "Canot find Ninja."
fi

#######################################################################################################################
# Generate the CMakePresets.json

# apple_os returns a name of the OS that corresponds to the given SDK.
apple_os() {
    case $1 in
        macosx)     echo macOS;;
        iphoneos)   echo iOS;;
        iphonesimulator)  echo iOS simulator;;
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

# generate_apple_cmake_presets generates a portion of cmake-variants.yaml for the given SDK and architectures.
generate_apple_cmake_presets() {
    SDK=$1
    shift
    for ARCH in $*; do
cat <<EOF
        {
            "name":         "${SDK}-${ARCH}-debug",
            "displayName":  "${SDK}-${ARCH}-debug",
            "description":  "",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA_EXECUTABLE}",
                "CMAKE_BUILD_TYPE":             "Debug",
                "CMAKE_CXX_COMPILER":           "$(xcrun --sdk ${SDK} --find clang++ 2>/dev/null)",
                "CMAKE_C_COMPILER":             "$(xcrun --sdk ${SDK} --find clang   2>/dev/null)",
                "CMAKE_OBJCXX_COMPILER":        "$(xcrun --sdk ${SDK} --find clang++ 2>/dev/null)",
                "CMAKE_OBJC_COMPILER":          "$(xcrun --sdk ${SDK} --find clang   2>/dev/null)",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "CMAKE_OSX_DEPLOYMENT_TARGET":  "$(apple_min_os_version ${SDK})",
                "CMAKE_OSX_SYSROOT":            "$(xcrun --sdk ${SDK} --show-sdk-path 2>/dev/null)"
            }
        },
        {
            "name":         "${SDK}-${ARCH}-release",
            "displayName":  "${SDK}-${ARCH}-release",
            "description":  "",
            "inherits":     "base",
            "architecture": {
                "value":    "${ARCH}",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM":           "${NINJA_EXECUTABLE}",
                "CMAKE_BUILD_TYPE":             "RelWithDebInfo",
                "CMAKE_CXX_COMPILER":           "$(xcrun --sdk ${SDK} --find clang++ 2>/dev/null)",
                "CMAKE_C_COMPILER":             "$(xcrun --sdk ${SDK} --find clang   2>/dev/null)",
                "CMAKE_OBJCXX_COMPILER":        "$(xcrun --sdk ${SDK} --find clang++ 2>/dev/null)",
                "CMAKE_OBJC_COMPILER":          "$(xcrun --sdk ${SDK} --find clang   2>/dev/null)",
                "CMAKE_OSX_ARCHITECTURES":      "${ARCH}",
                "CMAKE_OSX_DEPLOYMENT_TARGET":  "$(apple_min_os_version ${SDK})",
                "CMAKE_OSX_SYSROOT":            "$(xcrun --sdk ${SDK} --show-sdk-path 2>/dev/null)"
            }
        },
EOF
    done
}

# Will generate CMakePresets.json that is specific to your development environment, so that VSCode will be able
# to configure VSCode CMake Tools correctly.
cat <<EOF >${PROJECT_ROOT}/CMakePresets.json
{
    "version": 3,
    "configurePresets": [
$(generate_apple_cmake_presets     macosx          arm64       x86_64                      )
$(generate_apple_cmake_presets     iphoneos        arm64                                   )
$(generate_apple_cmake_presets     iphonesimulator arm64       x86_64                      )
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