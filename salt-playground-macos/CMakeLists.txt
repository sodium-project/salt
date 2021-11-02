find_package(imgui REQUIRED)

salt_macosx_app(playground_macosx
                SOURCES "playground.mm"
                LINK    salt::core
                        imgui::imgui
                        "-framework MetalKit")

# code: language="CMake" insertSpaces=true tabSize=4