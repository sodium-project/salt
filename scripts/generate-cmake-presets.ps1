Set-StrictMode -Version 2.0

function generate_cmake_presets($os, $arch, $graphics) {
    if ([string]::IsNullOrEmpty($env:llvm)) {
        throw @"
        Cannot find a path to the 'Clang' compiler, it looks like it is not set in the `$env`
        environment variable. Please add the path to the compiler by adding a new environment
        variable 'LLVM'. For example 'LLVM=C:/Program Files/LLVM/bin'.
"@
    }

    $description = ""
    if ($graphics -eq "DirectX") {
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

Add-Content -Path "../CMakePresets.json" -Value @"
{
    "version": 3,
    "configurePresets": [
        $(generate_cmake_presets "windows" "x86_64" "OpenGL")
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