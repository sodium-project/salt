#pragma once
#include <salt/foundation/heap_allocator.hpp>
#include <salt/foundation/static_allocator.hpp>

namespace salt::fdn {
// The default allocator that will be used as `BlockAllocator` in memory arenas.
// Arena allocators like `memory_stack` or `memory_pool` allocate memory by sub-
// dividing a huge block. They get a block allocator that will be used for their
// internal allocation, this type is the default value.
// The default is Heap_allocator.
using default_allocator = heap_allocator;
} // namespace salt::fdn