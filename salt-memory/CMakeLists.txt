# Generate containers_node_size.hpp if necessary
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/salt/memory/detail/containers_node_size.inl)
    salt_configure_containers(${CMAKE_CURRENT_BINARY_DIR}/salt/memory/detail/containers_node_size.inl)
endif()

salt_static_library(memory
    COMMON
        SOURCE
            "salt/memory/detail/debug_helpers.cpp"
            "salt/memory/detail/memory_list.cpp"
            "salt/memory/debugging.cpp"
            "salt/memory/temporary_allocator.cpp"
        TEST
            "salt/memory/detail/align-test.cpp"
            "salt/memory/detail/debug_helpers-test.cpp"
            "salt/memory/detail/fixed_memory_stack-test.cpp"
            "salt/memory/detail/memory_list_array-test.cpp"
            "salt/memory/detail/memory_list-test.cpp"
            "salt/memory/allocator_storage-test.cpp"
            "salt/memory/containers-test.cpp"
            "salt/memory/static_allocator-test.cpp"
            "salt/memory/heap_allocator-test.cpp"
            "salt/memory/memory_arena-test.cpp"
            "salt/memory/memory_pool-test.cpp"
            "salt/memory/memory_pool_list-test.cpp"
            "salt/memory/memory_stack-test.cpp"
            "salt/memory/smart_ptr-test.cpp"
            "salt/memory/std_allocator-test.cpp"
            "salt/memory/temporary_allocator-test.cpp"
        INCLUDE_DIR
            "${CMAKE_CURRENT_BINARY_DIR}"
        LINK
            salt::meta
            salt::foundation)

# code: language="CMake" insertSpaces=true tabSize=4