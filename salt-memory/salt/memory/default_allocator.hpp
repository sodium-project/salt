#pragma once
#include <salt/memory/heap_allocator.hpp>
#include <salt/memory/static_allocator.hpp>

namespace salt {
// The default is Heap_allocator.
using Default_allocator = Heap_allocator;
} // namespace salt
