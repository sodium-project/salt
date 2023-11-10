find_package(fast_io REQUIRED)

salt_static_library(foundation
    COMMON
        TEST
            # algorithm sub-module tests
            "salt/foundation/algorithm/equal-test.cpp"
            "salt/foundation/algorithm/lexicographical_compare-test.cpp"
            "salt/foundation/algorithm/minmax-test.cpp"

            # containers sub-module tests
            "salt/foundation/containers/array-test.cpp"
            "salt/foundation/containers/static_vector-test.cpp"

            # io sub-module tests
            "salt/foundation/io/manipulators-test.cpp"
            "salt/foundation/io/print-test.cpp"

            # memory sub-module tests
            "salt/foundation/memory/detail/constexpr_memcpy-test.cpp"
            "salt/foundation/memory/detail/debug_helpers-test.cpp"
            "salt/foundation/memory/detail/fixed_stack-test.cpp"
            "salt/foundation/memory/detail/free_list_array-test.cpp"
            "salt/foundation/memory/detail/free_list-test.cpp"
            "salt/foundation/memory/align-test.cpp"
            "salt/foundation/memory/heap_allocator-test.cpp"
            "salt/foundation/memory/memory_arena-test.cpp"
            "salt/foundation/memory/memory_pool-test.cpp"
            "salt/foundation/memory/memory_pool_list-test.cpp"
            "salt/foundation/memory/static_allocator-test.cpp"
            "salt/foundation/memory/uninitialized_construct-test.cpp"
            "salt/foundation/memory/uninitialized_storage-test.cpp"

            # utility sub-module tests
            "salt/foundation/utility/iterator_adapter-test.cpp"
        SOURCE
            # memory sub-module sources
            "salt/foundation/memory/debugging.cpp"
            "salt/foundation/memory/detail/debug_helpers.cpp"
        LINK
            fast_io::fast_io
            salt::platform)

# code: language="CMake" insertSpaces=true tabSize=4