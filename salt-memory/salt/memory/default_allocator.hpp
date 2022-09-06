#pragma once
#include <salt/memory/heap_allocator.hpp>
#include <salt/memory/static_allocator.hpp>

namespace salt {
// NOTE:
//  The default allocator that will be used as BlockAllocator in memory arenas. Arena allocators
//  like  Memory_stack or Memory_pool allocate memory by subdividing a huge block. They get a
//  BlockAllocator that will be used for their internal allocation, this type is the default value.
//  The default is Heap_allocator.
using Default_allocator = Heap_allocator;
} // namespace salt
