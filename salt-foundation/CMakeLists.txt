find_package(fast_io REQUIRED)

salt_static_library(foundation
    COMMON
        TEST
            # algorithm sub-module tests
            "salt/algorithm/equal-test.cpp"
            "salt/algorithm/lexicographical_compare-test.cpp"
            "salt/algorithm/minmax-test.cpp"

            # containers sub-module tests
            "salt/containers/array-test.cpp"
            "salt/containers/static_vector-test.cpp"

            # memory sub-module tests
            "salt/memory/detail/constexpr_memcpy-test.cpp"
            "salt/memory/detail/debug_helpers-test.cpp"
            "salt/memory/detail/fixed_stack-test.cpp"
            "salt/memory/detail/free_list_array-test.cpp"
            "salt/memory/detail/free_list-test.cpp"
            "salt/memory/align-test.cpp"
            "salt/memory/heap_allocator-test.cpp"
            "salt/memory/memory_arena-test.cpp"
            "salt/memory/static_allocator-test.cpp"
            "salt/memory/uninitialized_construct-test.cpp"
            "salt/memory/uninitialized_storage-test.cpp"

            # utility sub-module tests
            "salt/utility/iterator_adapter-test.cpp"
        SOURCE
            # memory sub-module sources
            "salt/memory/debugging.cpp"
            "salt/memory/detail/debug_helpers.cpp"
        LINK
            fast_io::fast_io
            salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4