find_package(fmt REQUIRED)

salt_interface_library(foundation
    COMMON
        TEST
            "salt/foundation/detail/as_local-test.cpp"
            "salt/foundation/detail/to_string-test.cpp"
            "salt/foundation/uninitialized_storage-test.cpp"
            "salt/foundation/static_storage-test.cpp"
            "salt/foundation/slot_map-test.cpp"
            "salt/foundation/detail/source_location-test.cpp"
        LINK
            fmt::fmt
            salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4