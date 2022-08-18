salt_static_library(memory
    COMMON
        SOURCE
            "salt/memory/debugging.cpp"
            "salt/memory/detail/debug_helpers.cpp"
            "salt/memory/detail/free_memory_list.cpp"
        TEST
            "salt/memory/detail/align-test.cpp"
            "salt/memory/detail/debug_helpers-test.cpp"
            "salt/memory/detail/memory_stack-test.cpp"
            "salt/memory/detail/free_memory_list-test.cpp"
        LINK
            salt::meta
            salt::foundation)

# code: language="CMake" insertSpaces=true tabSize=4