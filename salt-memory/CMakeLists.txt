salt_static_library(memory
    COMMON
        SOURCE
            "salt/memory/debugging.cpp"
            "salt/memory/detail/debug_helpers.cpp"
            "salt/memory/detail/memory_list.cpp"
        TEST
            "salt/memory/detail/align-test.cpp"
            "salt/memory/detail/debug_helpers-test.cpp"
            "salt/memory/detail/fixed_memory_stack-test.cpp"
            "salt/memory/detail/memory_list_array-test.cpp"
            "salt/memory/detail/memory_list-test.cpp"
            "salt/memory/static_allocator-test.cpp"
            "salt/memory/heap_allocator-test.cpp"
            "salt/memory/memory_arena-test.cpp"
            "salt/memory/memory_pool-test.cpp"
            "salt/memory/memory_pool_list-test.cpp"
        LINK
            salt::meta
            salt::foundation)

# code: language="CMake" insertSpaces=true tabSize=4