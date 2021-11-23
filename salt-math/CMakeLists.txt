find_package(glm REQUIRED)

salt_interface_library(math
                       COMMON_TEST "salt/math/compare-test.cpp"
                       PUBLIC_LINK glm::glm)

# code: language="CMake" insertSpaces=true tabSize=4