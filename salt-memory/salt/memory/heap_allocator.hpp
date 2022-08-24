#pragma once

#include <salt/memory/detail/low_level_allocator.hpp>
#include <salt/memory/detail/win32_heap_allocator.hpp>

namespace salt {

namespace detail {
using Heap_allocator_impl = detail::Win32_heap_allocator;
// clang-format off
[[maybe_unused]]
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(Heap_allocator_impl, heap_allocator_leak_detector)
// clang-format on
} // namespace detail

using Heap_allocator = detail::Low_level_allocator<detail::Heap_allocator_impl>;

} // namespace salt