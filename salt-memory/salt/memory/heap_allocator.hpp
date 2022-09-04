#pragma once
#include <salt/memory/detail/low_level_allocator.hpp>

#if defined(SALT_USE_MIMALLOC)
#    include <salt/memory/detail/mimalloc_allocator.hpp>
namespace salt::detail {
using Heap_allocator_impl = Mimalloc_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(Heap_allocator_impl, heap_allocator_leak_detector)
} // namespace salt::detail
#elif SALT_TARGET(WINDOWS)
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
#else
#    include <salt/foundation/fast_terminate.hpp>
#    include <salt/memory/debugging.hpp>
namespace salt::detail {
struct [[nodiscard]] Malloc_allocator final {
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static inline auto info() noexcept {
        return Allocator_info{"salt::detail::Malloc_allocator", nullptr};
    }

#    if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#    endif
    static inline void* allocate(size_type size, size_type) noexcept {
#    if __has_builtin(__builtin_malloc)
        void* memory = __builtin_malloc(size);
#    else
        void* memory = std::malloc(size);
#    endif
        if (!memory)
            salt::fast_terminate();
        return memory;
    }

    static inline void deallocate(void* memory, size_type, size_type) noexcept {
        if (!memory) [[unlikely]]
            return;

#    if __has_builtin(__builtin_free)
        __builtin_free(memory);
#    else
        std::free(memory);
#    endif
    }

    static inline size_type max_size() noexcept {
        // The maximum size of a user request for memory that can be granted.
        return size_type(-1) / sizeof(std::byte);
    }
};

using Heap_allocator_impl = Malloc_allocator;
SALT_MEMORY_LL_ALLOCATOR_LEAK_HANDLER(Heap_allocator_impl, heap_allocator_leak_detector)
} // namespace salt::detail
#endif

namespace salt {
using Heap_allocator = detail::Low_level_allocator<detail::Heap_allocator_impl>;
} // namespace salt