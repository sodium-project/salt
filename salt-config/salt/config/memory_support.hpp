#pragma once

// Whether or not the allocation size will be checked.
#define SALT_MEMORY_CHECK_ALLOCATION_SIZE (1)

// Whether or not allocated memory will be filled with special values.
#define SALT_MEMORY_DEBUG_FILL (1)

// The size of the fence memory, it has no effect if SALT_MEMORY_DEBUG_FILL is disabled.
#define SALT_MEMORY_DEBUG_FENCE (1)

// Whether or not leak checking is enabled.
#define SALT_MEMORY_DEBUG_LEAK (1)

// Whether or not the deallocation functions will check for pointers that were never allocated by an
// allocator.
#define SALT_MEMORY_DEBUG_POINTER (1)

// Whether or not the deallocation functions will check for double free errors.
#define SALT_MEMORY_DEBUG_DOUBLE_FREE (1)

// Whether or not the `Temporary_allocator` will use a `Temporary_stack` for its allocation. This
// option controls how and if a global, per-thread instance of it is managed. If 2 it is
// automatically managed and created on-demand, if 1 you need explicit lifetime control through the
// `Temporary_stack_initializer` class and if 0 there is no stack created automatically. Mode 2 has
// a slight runtime overhead.
#define SALT_MEMORY_TEMPORARY_STACK_MODE (2)

#ifdef NDEBUG
#    undef SALT_MEMORY_CHECK_ALLOCATION_SIZE
#    define SALT_MEMORY_CHECK_ALLOCATION_SIZE (0)

#    undef SALT_MEMORY_DEBUG_FILL
#    define SALT_MEMORY_DEBUG_FILL (0)

#    undef SALT_MEMORY_DEBUG_FENCE
#    define SALT_MEMORY_DEBUG_FENCE (0)

#    undef SALT_MEMORY_DEBUG_LEAK
#    define SALT_MEMORY_DEBUG_LEAK (0)

#    undef SALT_MEMORY_DEBUG_POINTER
#    define SALT_MEMORY_DEBUG_POINTER (0)

#    undef SALT_MEMORY_DEBUG_DOUBLE_FREE
#    define SALT_MEMORY_DEBUG_DOUBLE_FREE (0)
#endif