find_package(imgui REQUIRED)

enable_language(OBJCXX)

salt_macosx_app(playground
                SOURCES "playground.mm"
                LINK    salt::core
                        imgui::imgui
                        "-framework MetalKit")

# code: language="CMake" insertSpaces=true tabSize=4