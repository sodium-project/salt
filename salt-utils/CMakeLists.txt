find_package(fmt REQUIRED)

salt_interface_library(utils
    COMMON
        TEST
            "salt/utils/as_local-test.cpp"
            "salt/utils/to_string-test.cpp"
            "salt/utils/uninitialized_storage-test.cpp"
            "salt/utils/static_storage-test.cpp"
            "salt/utils/detail/source_location-test.cpp"
        LINK
            fmt::fmt
            salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4