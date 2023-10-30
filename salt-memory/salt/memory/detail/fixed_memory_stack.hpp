#pragma once
#include <salt/memory/align.hpp>
#include <salt/memory/debugging.hpp>
#include <salt/memory/detail/debug_helpers.hpp>
#include <salt/memory/detail/utility.hpp>

namespace salt::memory::detail {

// Simple memory stack implementation that does not support growing.
struct [[nodiscard]] fixed_memory_stack final {
    constexpr fixed_memory_stack() noexcept : current_{nullptr} {}

    constexpr explicit fixed_memory_stack(void* const memory) noexcept
            : current_{static_cast<std::byte*>(memory)} {}

    constexpr fixed_memory_stack(fixed_memory_stack&& other) noexcept
            : current_{detail::exchange(other.current_, nullptr)} {}

    constexpr ~fixed_memory_stack() = default;

    constexpr fixed_memory_stack& operator=(fixed_memory_stack&& other) noexcept {
        current_ = detail::exchange(other.current_, nullptr);
        return *this;
    }

    constexpr void advance(std::size_t offset) noexcept {
        current_ += offset;
    }

    constexpr void advance(std::size_t offset, debug_magic magic_value) noexcept {
        debug_fill(current_, offset, magic_value);
        advance(offset);
    }

    constexpr void* advance_return(std::size_t offset,
                                   debug_magic magic_value = debug_magic::new_memory) noexcept {
        auto memory = current_;
        debug_fill(memory, offset, magic_value);
        advance(offset);
        return memory;
    }

    constexpr void* allocate(std::byte const* end, std::size_t size, std::size_t alignment,
                             std::size_t fence_size = debug_fence_size) noexcept {
        if (current_ == nullptr)
            return current_;

        auto const remaining = static_cast<std::size_t>(end - current_);
        auto const offset    = align_offset(current_ + fence_size, alignment);
        if (fence_size + offset + size + fence_size > remaining)
            return nullptr;

        return allocate_unchecked(size, offset, fence_size);
    }

    constexpr void* allocate_unchecked(std::size_t size, std::size_t align_offset,
                                       std::size_t fence_size = debug_fence_size) noexcept {
        advance(fence_size, debug_magic::fence_memory);
        advance(align_offset, debug_magic::alignment_memory);
        auto* memory = advance_return(size);
        advance(fence_size, debug_magic::fence_memory);
        return memory;
    }

    constexpr void unwind(std::byte* top) noexcept {
        debug_fill(top, static_cast<std::size_t>(current_ - top), debug_magic::freed_memory);
        current_ = top;
    }

    constexpr std::byte* top() const noexcept {
        return current_;
    }

private:
    std::byte* current_;
};

} // namespace salt::memory::detail