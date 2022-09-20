#pragma once
#include <salt/memory/debugging.hpp>
#include <salt/memory/detail/align.hpp>
#include <salt/memory/detail/debug_helpers.hpp>

namespace salt::detail {

struct [[nodiscard]] Fixed_memory_stack final {
    constexpr Fixed_memory_stack() noexcept : current_{nullptr} {}

    constexpr explicit Fixed_memory_stack(void* memory) noexcept
            : current_{static_cast<std::byte*>(memory)} {}

    constexpr Fixed_memory_stack(Fixed_memory_stack&& other) noexcept
            : current_{std::exchange(other.current_, nullptr)} {}

    constexpr ~Fixed_memory_stack() = default;

    constexpr Fixed_memory_stack& operator=(Fixed_memory_stack&& other) noexcept {
        current_ = std::exchange(other.current_, nullptr);
        return *this;
    }

    constexpr void shift(std::size_t offset) noexcept {
        current_ += offset;
    }

    constexpr void shift(std::size_t offset, debug_magic magic_value) noexcept {
        debug_fill(current_, offset, magic_value);
        shift(offset);
    }

    constexpr void* shift_top(std::size_t offset,
                              debug_magic magic_value = debug_magic::new_memory) noexcept {
        auto memory = current_;
        debug_fill(memory, offset, magic_value);
        shift(offset);
        return memory;
    }

    constexpr void* allocate(std::byte const* end, std::size_t size, std::size_t alignment,
                             std::size_t fence_size = debug_fence_size) noexcept {
        if (current_ == nullptr)
            return nullptr;

        auto remaining = std::size_t(end - current_);
        auto offset    = align_offset(current_ + fence_size, alignment);
        if (fence_size + offset + size + fence_size > remaining)
            return nullptr;

        return allocate_unchecked(size, offset, fence_size);
    }

    constexpr void* allocate_unchecked(std::size_t size, std::size_t align_offset,
                                       std::size_t fence_size = debug_fence_size) noexcept {
        shift(fence_size, debug_magic::fence_memory);
        shift(align_offset, debug_magic::alignment_memory);
        auto memory = shift_top(size);
        shift(fence_size, debug_magic::fence_memory);
        return memory;
    }

    constexpr void unwind(std::byte* top) noexcept {
        debug_fill(top, std::size_t(current_ - top), debug_magic::freed_memory);
        current_ = top;
    }

    constexpr std::byte* top() const noexcept {
        return current_;
    }

private:
    std::byte* current_;
};

using Memory_stack = Fixed_memory_stack;

} // namespace salt::detail
