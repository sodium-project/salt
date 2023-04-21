#pragma once

#include <salt/config.hpp>
#include <salt/memory/allocator_storage.hpp>
#include <salt/memory/threading.hpp>

namespace salt {

// Deallocator deallocates memory for a specified type but does not call its destructors.
template <typename T, raw_allocator RawAllocator> requires(not std::is_abstract_v<T>)
struct [[nodiscard]] Deallocator final : Allocator_reference<RawAllocator> {
    using allocator_type  = typename Allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr Deallocator() noexcept = default;

    constexpr explicit Deallocator(Allocator_reference<RawAllocator> allocator) noexcept
            : Allocator_reference<RawAllocator>{allocator} {}

    constexpr void operator()(pointer_type pointer) noexcept {
        this->deallocate_node(pointer, sizeof(value_type), alignof(value_type));
    }

    constexpr decltype(auto) allocator() const noexcept {
        return this->allocator();
    }
};

// Specialization of Deallocator for array types.
template <typename T, raw_allocator RawAllocator> requires(not std::is_abstract_v<T>)
struct [[nodiscard]] Deallocator<T[], RawAllocator> final : Allocator_reference<RawAllocator> {
    using allocator_type  = typename Allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr Deallocator() noexcept : size_{0u} {};

    constexpr explicit Deallocator(Allocator_reference<RawAllocator> allocator) noexcept
            : Allocator_reference<RawAllocator>{allocator}, size_{0u} {}

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

// Similar to Deallocator but calls the destructors of the object.
template <typename T, raw_allocator RawAllocator> requires(not std::is_abstract_v<T>)
struct [[nodiscard]] Deleter : Allocator_reference<RawAllocator> {
    using allocator_type  = typename Allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr Deleter() noexcept = default;

    constexpr explicit Deleter(Allocator_reference<RawAllocator> allocator) noexcept
            : Allocator_reference<RawAllocator>{allocator} {}

    constexpr void operator()(pointer_type pointer) noexcept {
        pointer->~value_type();
        this->deallocate_node(pointer, sizeof(value_type), alignof(value_type));
    }

    constexpr decltype(auto) allocator() const noexcept {
        return this->allocator();
    }
};

// Specialization of Deleter for array types.
template <typename T, raw_allocator RawAllocator> requires(not std::is_abstract_v<T>)
struct [[nodiscard]] Deleter<T[], RawAllocator> final : Allocator_reference<RawAllocator> {
    using allocator_type  = typename Allocator_reference<RawAllocator>::allocator_type;
    using value_type      = T;
    using pointer_type    = T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    constexpr Deleter() noexcept : size_{0u} {};

    constexpr explicit Deleter(Allocator_reference<RawAllocator> allocator) noexcept
            : Allocator_reference<RawAllocator>{allocator}, size_{0u} {}

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

} // namespace salt
