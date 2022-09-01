#pragma once
#include <cstddef>
#include <cstdint>

#include <salt/foundation/fast_terminate.hpp>
#include <salt/memory/debugging.hpp>

namespace salt::detail {

namespace linux {

extern void* kernel_kmalloc(std::size_t, std::uint32_t) noexcept __asm__("__kmalloc");

extern void kernel_kfree(void const*) noexcept __asm__("kfree");

constexpr inline std::uint32_t gfp_kernel      = 0x400u | 0x800u | 0x40u | 0x80u;
constexpr inline std::uint32_t gfp_kernel_zero = gfp_kernel | 0x100u;

} // namespace linux

// clang-format off
struct [[nodiscard]] Linux_kmalloc_allocator final {

    static inline Allocator_info info() noexcept {
        return {"salt::detail::Linux_kmalloc_allocator", nullptr};
    }

#if __has_cpp_attribute(__gnu__::__returns_nonnull__)
    [[__gnu__::__returns_nonnull__]]
#endif
    static inline void* allocate(std::size_t size, std::size_t) noexcept {
        if (size == 0) [[unlikely]]
            size = 1;

        void* memory = linux::kernel_kmalloc(size, linux::gfp_kernel);
        if (!memory)
            salt::fast_terminate();
        return memory;
    }

    static inline void deallocate(void* ptr, std::size_t, std::size_t) noexcept {
        if (!ptr) [[unlikely]]
            return;

        linux::kernel_kfree(ptr);
    }

    static inline std::size_t max_size() noexcept {
        // The maximum size of a user request for memory that can be granted on x64.
        return 1ULL << 23;
    }
};
// clang-format on

} // namespace salt::detail