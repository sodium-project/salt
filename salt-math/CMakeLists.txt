find_package(glm REQUIRED)

salt_interface_library(math
                       COMMON_TEST "salt/math/compare-test.cpp"
                                   "salt/math/primitives-test.cpp"
                       PUBLIC_LINK glm::glm
                                   salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4