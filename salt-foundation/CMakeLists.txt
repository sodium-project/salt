salt_static_library(foundation
    COMMON
        TEST
            # containers tests
            "salt/foundation/array-test.cpp"
            "salt/foundation/static_vector-test.cpp"

            # memory tests
            "salt/foundation/detail/constexpr_memcpy-test.cpp"
            "salt/foundation/detail/debug_helpers-test.cpp"
            "salt/foundation/detail/fixed_stack-test.cpp"
            "salt/foundation/detail/free_list_array-test.cpp"
            "salt/foundation/detail/free_list-test.cpp"
            "salt/foundation/align-test.cpp"
            "salt/foundation/allocator_storage-test.cpp"
            "salt/foundation/heap_allocator-test.cpp"
            "salt/foundation/memory_arena-test.cpp"
            "salt/foundation/memory_pool-test.cpp"
            "salt/foundation/memory_pool_list-test.cpp"
            "salt/foundation/memory_stack-test.cpp"
            "salt/foundation/static_allocator-test.cpp"
            "salt/foundation/temporary_allocator-test.cpp"
            "salt/foundation/std_allocator_adapter-test.cpp"
            "salt/foundation/uninitialized_construct-test.cpp"
            "salt/foundation/uninitialized_storage-test.cpp"

            # utility tests
            "salt/foundation/iterator_adapter-test.cpp"
        SOURCE
            # memory sources
            "salt/foundation/debugging.cpp"
            "salt/foundation/detail/debug_helpers.cpp"
            "salt/foundation/temporary_allocator.cpp"
        LINK
            salt::logging
            salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4