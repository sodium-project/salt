find_package(fast_io REQUIRED)

salt_interface_library(foundation
    COMMON
        TEST
            "salt/foundation/uninitialized_storage-test.cpp"
        LINK
            salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4