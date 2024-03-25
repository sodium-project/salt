find_package(fast_io REQUIRED)

salt_interface_library(io
    COMMON
        TEST
            "salt/io/manipulators-test.cpp"
            "salt/io/print-test.cpp"
        LINK
            fast_io::fast_io
            salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4