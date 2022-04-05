find_package(glm REQUIRED)

salt_interface_library(math
    COMMON
        TEST
          "salt/math/compare-test.cpp"
          "salt/math/primitives-test.cpp"
        LINK
          glm::glm
          salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4