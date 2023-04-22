find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

salt_common_app(common_playground
                SOURCES "common_playground.cpp"
                LINK    salt::core
                        imgui::imgui)

salt_macosx_app(macos_playground
                SOURCES "macos_playground.mm"
                LINK    salt::core
                        imgui::imgui
                        "-framework MetalKit")

# code: language="CMake" insertSpaces=true tabSize=4