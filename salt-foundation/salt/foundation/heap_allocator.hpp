#pragma once
#include <salt/config.hpp>
#include <salt/foundation/detail/low_level_allocator_adapter.hpp>

#if defined(SALT_USE_MIMALLOC)
// TODO:
//  Implement `mimalloc` allocator support.
#elif SALT_TARGET(WINDOWS)
#    include <salt/foundation/detail/win32_heap_allocator.hpp>
namespace salt::fdn::detail {
using heap_allocator_impl = win32_heap_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(heap_allocator_impl, heap_allocator_leak_detector);
} // namespace salt::fdn::detail
#else
#    include <salt/foundation/detail/malloc_allocator.hpp>
namespace salt::fdn::detail {
using heap_allocator_impl = malloc_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(heap_allocator_impl, heap_allocator_leak_detector);
} // namespace salt::fdn::detail
#endif

namespace salt::fdn {
using heap_allocator = detail::low_level_allocator_adapter<detail::heap_allocator_impl>;
} // namespace salt::fdn