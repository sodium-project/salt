salt_static_library(memory
    COMMON
        TEST
            "salt/memory/detail/constexpr_memcpy-test.cpp"
            "salt/memory/detail/debug_helpers-test.cpp"
            "salt/memory/detail/fixed_stack-test.cpp"
            "salt/memory/detail/free_list-test.cpp"
            "salt/memory/align-test.cpp"
            "salt/memory/static_allocator-test.cpp"
            "salt/memory/uninitialized_construct-test.cpp"
            "salt/memory/uninitialized_storage-test.cpp"
        SOURCE
            "salt/memory/dummy.cpp"
            "salt/memory/debugging.cpp"
            "salt/memory/detail/debug_helpers.cpp"
        LINK
            salt::logging
            salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4