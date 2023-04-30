Set-StrictMode -Version 2.0

function detect_clang() {
    if ([string]::IsNullOrEmpty($env:llvm)) {
        throw @"
        Cannot find a path to the 'Clang' compiler, it looks like it is not set in the `$env`
        environment variable. Please add the path to the compiler by adding a new environment
        variable 'LLVM'. For example 'LLVM=C:/Program Files/LLVM/bin'.
"@
    }

    $clang_version_major = Write-Output '__clang_major__'       | clang -E -x c - | Select-Object -Last 1
    $clang_version_minor = Write-Output '__clang_minor__'       | clang -E -x c - | Select-Object -Last 1
    $clang_version_patch = Write-Output '__clang_patchlevel__ ' | clang -E -x c - | Select-Object -Last 1

    if ([int]$clang_version_major -lt 16) {
        throw @"
        The minimum supported version of Clang is 16 or higher. Please update Clang to the minimum supported version.
"@
    }

    Write-Output "Detected clang version: $clang_version_major.$clang_version_minor.$clang_version_patch"
}

function generate_cmake_presets($os, $arch, $graphics) {
    $description = ""
    if ($graphics -ne "OpenGL") {
        $description = "This configuration is temporarily unsupported."
    }
    $presets = @"
        {
            "name":         "$os-$arch-$($graphics.ToLower())-debug",
            "displayName":  "$os-$arch-$($graphics.ToLower())-debug",
            "description":  "$description",
            "inherits":     "base",
            "architecture": {
                "value":    "$arch",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_BUILD_TYPE":   "Debug",
                "CMAKE_C_COMPILER":   "$env:llvm/clang.exe",
                "CMAKE_CXX_COMPILER": "$env:llvm/clang++.exe",
                "CMAKE_RC_COMPILER":  "$env:llvm/clang++.exe",
                "SALT_GRAPHICS":      "$graphics"
            }
        },
        {
            "name":         "$os-$arch-$($graphics.ToLower())-release",
            "displayName":  "$os-$arch-$($graphics.ToLower())-release",
            "description":  "$description",
            "inherits":     "base",
            "architecture": {
                "value":    "$arch",
                "strategy": "external"
            },
            "cacheVariables": {
                "CMAKE_BUILD_TYPE":   "RelWithDebInfo",
                "CMAKE_C_COMPILER":   "$env:llvm/clang.exe",
                "CMAKE_CXX_COMPILER": "$env:llvm/clang++.exe",
                "CMAKE_RC_COMPILER":  "$env:llvm/clang++.exe",
                "SALT_GRAPHICS":      "$graphics"
            }
        },
"@
    return $presets
}

$(detect_clang)

$project_root = split-path $MyInvocation.MyCommand.Definition | split-path -parent
$cmake_presets = "$project_root\CMakePresets.json"

if (Test-Path $cmake_presets) {
    Remove-Item $cmake_presets
}

Write-Output "Generating presets.."

Add-Content -Path $cmake_presets -Value @"
{
    "version": 3,
    "configurePresets": [
$(generate_cmake_presets "windows" "x86_64" "OpenGL")
$(generate_cmake_presets "windows" "x86_64" "Vulkan")
$(generate_cmake_presets "windows" "x86_64" "DirectX")
        {
            "name":        "base",
            "description": "For more information: http://aka.ms/cmakepresetsvs",
            "hidden":      true,
            "generator":   "Ninja",
            "binaryDir":   "`${sourceDir}/build/`${presetName}",
            "installDir":  "`${sourceDir}/build/`${presetName}/output/"
        }
    ]
}
"@

Write-Output "Successfully generated presets in $cmake_presets"