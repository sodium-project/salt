#pragma once
#include <salt/memory/debugging.hpp>

namespace salt::detail {

namespace utils {

// A load operation copies data from a pointer to an integer.
inline std::uintptr_t load_int(void* address) noexcept {
    SALT_ASSERT(address);
    std::uintptr_t value;
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(&value, address, sizeof(std::uintptr_t));
#else
    std::memcpy(&value, address, sizeof(std::uintptr_t));
#endif
    return value;
}

// A store operation copies data from an integer to a pointer.
inline void store_int(void* address, std::uintptr_t value) noexcept {
    SALT_ASSERT(address);
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy(address, &value, sizeof(std::uintptr_t));
#else
    std::memcpy(address, &value, sizeof(std::uintptr_t));
#endif
}

#if __has_cpp_attribute(__gnu__::__always_inline__)
[[__gnu__::__always_inline__]]
#elif __has_cpp_attribute(msvc::forceinline)
[[msvc::forceinline]]
#endif
inline std::uintptr_t
to_int(std::byte* ptr) noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr);
}

inline std::byte* load(void* address) noexcept {
    return reinterpret_cast<std::byte*>(load_int(address));
}

inline void store(void* address, std::byte* ptr) noexcept {
    store_int(address, to_int(ptr));
}

inline std::byte* xor_load(void* address, std::byte* prev_or_next) noexcept {
    return reinterpret_cast<std::byte*>(load_int(address) ^ to_int(prev_or_next));
}

inline void xor_store(void* address, std::byte* prev, std::byte* next) noexcept {
    store_int(address, to_int(prev) ^ to_int(next));
}

inline void xor_exchange(void* address, std::byte* old_ptr, std::byte* new_ptr) noexcept {
    auto other = xor_load(address, old_ptr);
    xor_store(address, other, new_ptr);
}

inline void xor_next(std::byte*& current, std::byte*& prev) noexcept {
    auto next = xor_load(current, prev);
    prev      = current;
    current   = next;
}

} // namespace utils

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

} // namespace salt::detail