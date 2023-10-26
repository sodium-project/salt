find_package(fast_io REQUIRED)

salt_interface_library(logging
    COMMON
        LINK
            fast_io::fast_io
            salt::meta)

# code: language="CMake" insertSpaces=true tabSize=4