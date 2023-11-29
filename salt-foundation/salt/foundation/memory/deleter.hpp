#pragma once

#include <salt/config.hpp>
#include <salt/foundation/memory/allocator_storage.hpp>

namespace salt::memory {

// `Deallocator` deallocates memory for a specified type but does not call its destructors.
template <meta::not_abstract T, raw_allocator RawAllocator>
struct [[nodiscard]] deallocator final : allocator_reference<RawAllocator> {
    using allocator_type  = typename allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr deallocator() noexcept = default;

    constexpr explicit(false) deallocator(allocator_reference<RawAllocator> allocator) noexcept
            : allocator_reference<RawAllocator>{allocator} {}

    constexpr void operator()(pointer_type pointer) noexcept {
        this->deallocate_node(pointer, sizeof(value_type), alignof(value_type));
    }

    constexpr decltype(auto) allocator() const noexcept {
        return this->allocator();
    }
};

// Specialization of `deallocator` for array types.
template <meta::not_abstract T, raw_allocator RawAllocator>
struct [[nodiscard]] deallocator<T[], RawAllocator> final : allocator_reference<RawAllocator> {
    using allocator_type  = typename allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr deallocator() noexcept : size_{0u} {};

    constexpr explicit(false) deallocator(allocator_reference<RawAllocator> allocator) noexcept
            : allocator_reference<RawAllocator>{allocator}, size_{0u} {}

    constexpr void operator()(pointer_type pointer) noexcept {
        this->deallocate_array(pointer, size_, sizeof(value_type), alignof(value_type));
    }

    constexpr decltype(auto) allocator() const noexcept {
        return this->allocator();
    }

    constexpr size_type array_size() const noexcept {
        return size_;
    }

private:
    size_type size_;
};

// Similar to `deallocator` but calls the destructors of the object.
template <meta::not_abstract T, raw_allocator RawAllocator>
struct [[nodiscard]] deleter : allocator_reference<RawAllocator> {
    using allocator_type  = typename allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr deleter() noexcept = default;

    constexpr explicit(false) deleter(allocator_reference<RawAllocator> allocator) noexcept
            : allocator_reference<RawAllocator>{allocator} {}

    constexpr void operator()(pointer_type pointer) noexcept {
        pointer->~value_type();
        this->deallocate_node(pointer, sizeof(value_type), alignof(value_type));
    }

    constexpr decltype(auto) allocator() const noexcept {
        return this->allocator();
    }
};

// Specialization of `deleter` for array types.
template <meta::not_abstract T, raw_allocator RawAllocator>
struct [[nodiscard]] deleter<T[], RawAllocator> final : allocator_reference<RawAllocator> {
    using allocator_type  = typename allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr deleter() noexcept : size_{0u} {};

    constexpr explicit(false) deleter(allocator_reference<RawAllocator> allocator) noexcept
            : allocator_reference<RawAllocator>{allocator}, size_{0u} {}

    constexpr void operator()(pointer_type pointer) noexcept {
        for (auto it = pointer; it != pointer + size_; ++it) {
            it->~value_type();
        }
        this->deallocate_array(pointer, size_, sizeof(value_type), alignof(value_type));
    }

    constexpr decltype(auto) allocator() const noexcept {
        return this->allocator();
    }

    constexpr size_type array_size() const noexcept {
        return size_;
    }

private:
    size_type size_;
};

} // namespace salt::memory