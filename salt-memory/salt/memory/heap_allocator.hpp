#pragma once

#include <salt/memory/detail/low_level_allocator.hpp>

#if (defined(_WIN32) || defined(__CYGWIN__)) && !defined(__WINE__)
#    include <salt/memory/detail/win32_heap_allocator.hpp>
#    if defined(_MSC_VER) && !defined(SALT_CLANG)
#        pragma comment(                                                                           \
                linker,                                                                            \
                "/alternatename:__imp_?HeapAlloc@win32@detail@salt@@YAPEAXPEAXI_K@Z=__imp_HeapAlloc")
#        pragma comment(                                                                           \
                linker,                                                                            \
                "/alternatename:__imp_?HeapFree@win32@detail@salt@@YAHPEAXI0@Z=__imp_HeapFree")
#        pragma comment(                                                                           \
                linker,                                                                            \
                "/alternatename:__imp_?GetProcessHeap@win32@detail@salt@@YAPEAXXZ=__imp_GetProcessHeap")
#    endif
namespace salt::detail {
using Heap_allocator_impl = Win32_heap_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(Heap_allocator_impl, heap_allocator_leak_detector)
} // namespace salt::detail
#endif

#if (defined(__linux__) && defined(__KERNEL__)) || defined(SALT_USE_LINUX_KERNEL_ALLOCATOR)
#    include <salt/memory/detail/linux_heap_allocator.hpp>
namespace salt::detail {
using Heap_allocator_impl = Linux_kmalloc_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(Heap_allocator_impl, heap_allocator_leak_detector)
} // namespace salt::detail
#endif

#if (!defined(_MSC_VER) || defined(SALT_CLANG)) && defined(SALT_USE_MIMALLOC)
#    include <salt/memory/detail/mimalloc_allocator.hpp>
namespace salt::detail {
using Heap_allocator_impl = Mimalloc_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(Heap_allocator_impl, heap_allocator_leak_detector)
} // namespace salt::detail
#endif

namespace salt {
using Heap_allocator = detail::Low_level_allocator<detail::Heap_allocator_impl>;
} // namespace salt