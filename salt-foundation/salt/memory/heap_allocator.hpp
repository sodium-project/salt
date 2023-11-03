#pragma once
#include <salt/memory/detail/low_level_allocator_adapter.hpp>

#if defined(SALT_USE_MIMALLOC)
// TODO:
//  Implement `mimalloc` allocator support.
#elif SALT_TARGET(WINDOWS)
#    include <salt/memory/detail/win32_heap_allocator.hpp>
namespace salt::memory::detail {
using heap_allocator_impl = win32_heap_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(heap_allocator_impl, heap_allocator_leak_detector);
} // namespace salt::memory::detail
#else
#    include <salt/memory/detail/malloc_allocator.hpp>
namespace salt::memory::detail {
using heap_allocator_impl = malloc_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(heap_allocator_impl, heap_allocator_leak_detector);
} // namespace salt::memory::detail
#endif

namespace salt::memory {
using heap_allocator = detail::low_level_allocator_adapter<detail::heap_allocator_impl>;
} // namespace salt::memory