#pragma once
#include <salt/config.hpp>
#include <salt/logging.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>

namespace salt::memory::detail {

// A load operation copies data from a pointer to an integer.
inline std::uintptr_t load_int(void* address) noexcept {
    SALT_ASSERT(address);
    std::uintptr_t value;
    __builtin_memcpy(&value, address, sizeof(std::uintptr_t));
    return value;
}

// A store operation copies data from an integer to a pointer.
inline void store_int(void* address, std::uintptr_t value) noexcept {
    SALT_ASSERT(address);
    __builtin_memcpy(address, &value, sizeof(std::uintptr_t));
}

// clang-format off
#if __has_cpp_attribute(clang::always_inline)
[[clang::always_inline]]
#endif
inline std::uintptr_t to_int(std::byte* ptr) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr);
}

#if __has_cpp_attribute(clang::always_inline)
[[clang::always_inline]]
#endif
inline std::byte* from_int(std::uintptr_t ptr) noexcept {
    return reinterpret_cast<std::byte*>(ptr);
}
// clang-format oт

inline std::byte* get_next(void* address) noexcept {
    return from_int(load_int(address));
}

inline void set_next(void* address, std::byte* next) noexcept {
    store_int(address, to_int(next));
}

inline std::byte* xor_get_next(void* address, std::byte* prev_or_next) noexcept {
    return from_int(load_int(address) ^ to_int(prev_or_next));
}

inline void xor_set_next(void* address, std::byte* prev, std::byte* next) noexcept {
    store_int(address, to_int(prev) ^ to_int(next));
}

inline void xor_advance(std::byte*& current, std::byte*& prev) noexcept {
    auto next = xor_get_next(current, prev);
    prev      = current;
    current   = next;
}

inline void xor_exchange(void* address, std::byte* old_ptr, std::byte* new_ptr) noexcept {
    auto next = xor_get_next(address, old_ptr);
    xor_set_next(address, next, new_ptr);
}

// clang-format off
template <typename T, typename Other = T>
constexpr T exchange(T& obj, Other&& new_value) noexcept {
    T old_value = static_cast<T&&>(obj);
    obj         = static_cast<Other&&>(new_value);
    return old_value;
}
// clang-format on

constexpr bool less(void* a, void* b) noexcept {
    return std::less<void*>()(a, b);
}

constexpr bool less_equal(void* a, void* b) noexcept {
    return a == b || less(a, b);
}

constexpr bool greater(void* a, void* b) noexcept {
    return std::greater<void*>()(a, b);
}

constexpr bool greater_equal(void* a, void* b) noexcept {
    return a == b || greater(a, b);
}

} // namespace salt::memory::detail