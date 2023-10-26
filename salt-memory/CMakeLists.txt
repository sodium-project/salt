salt_static_library(memory
    COMMON
        TEST
            "salt/memory/uninitialized_construct-test.cpp"
            "salt/memory/uninitialized_storage-test.cpp"
            "salt/memory/detail/constexpr_memcpy-test.cpp"
        SOURCE
            "salt/memory/dummy.cpp"
        LINK
            salt::logging
            salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4