find_package(fast_io REQUIRED)

salt_interface_library(foundation
    COMMON
        TEST
            "salt/foundation/algorithm-test.cpp"
            "salt/foundation/array-test.cpp"
            "salt/foundation/static_vector-test.cpp"
            "salt/foundation/detail/iterator_adapter-test.cpp"
        LINK
            salt::memory)

# code: language="CMake" insertSpaces=true tabSize=4