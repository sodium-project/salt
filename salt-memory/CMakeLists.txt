salt_static_library(memory
    COMMON
        SOURCE
            "salt/memory/debugging.cpp"
            "salt/memory/detail/debug_helpers.cpp"
            "salt/memory/detail/free_list.cpp"
        TEST
            "salt/memory/detail/align-test.cpp"
            "salt/memory/detail/debug_helpers-test.cpp"
            "salt/memory/detail/memory_stack-test.cpp"
            "salt/memory/detail/free_list-test.cpp"
            "salt/memory/detail/free_list_array-test.cpp"
            "salt/memory/static_allocator-test.cpp"
            "salt/memory/heap_allocator-test.cpp"
            "salt/memory/memory_arena-test.cpp"
        LINK
            salt::meta
            salt::foundation)

# code: language="CMake" insertSpaces=true tabSize=4